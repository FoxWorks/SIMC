////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2012-2013, Black Phoenix
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///   - Redistributions of source code must retain the above copyright
///     notice, this list of conditions and the following disclaimer.
///   - Redistributions in binary form must reproduce the above copyright
///     notice, this list of conditions and the following disclaimer in the
///     documentation and/or other materials provided with the distribution.
///   - Neither the name of the author nor the names of the contributors may
///     be used to endorse or promote products derived from this software without
///     specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
/// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
/// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
/// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
/// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
/// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
/// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include "sim_core.h"


////////////////////////////////////////////////////////////////////////////////
/// @brief Allocate a new linked list.
///
/// Multithreading support for lists means the following:
///	- Many threads may read lists simultaneously. Several threads are permitted
///		to read at once.
///	- Several threads may request adding a new element anytime. Only one thread
/// 	will add an element at any time.
///	- At all times, only a single thread is permitted to remove from the linked list.
///
/// See SIMC_List_Remove() for more information on restrictions regarding deleting from
/// a linked list. Same restrictions apply to SIMC_List_MoveInFront().
///
/// Without multithreading support only a single thread may add or remove items. If any
/// other thread accesses the list in the meantime, it will enter undefined state.
///
/// Single-threaded lists are useful if they are read-only and will not be modified
/// while other threads may access them. They do not produce SRW lock overhead.
///
/// @param[out] p_list Pointer to the linked list will be written here
/// @param[in] multithreaded Should multithreading support be enabled for this list
////////////////////////////////////////////////////////////////////////////////
void SIMC_List_Create(SIMC_LIST** p_list, int multithreaded) {
	SIMC_LIST* list = (SIMC_LIST*)malloc(sizeof(SIMC_LIST));
	list->first = 0;
	list->last = 0;
#ifndef SIMC_SINGLETHREADED
	list->lock = SIMC_THREAD_BAD_ID;
	if (multithreaded) list->lock = SIMC_SRW_Create();
#endif
	*p_list = list;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Destroy the linked list.
///
/// Requests and exclusive write lock. When no other threads are reading from the list,
/// the calling thread will destroy the list.
///
/// Only list structure is destroyed, the data represented by the list is not.
///
/// @param[in] list Pointer to the linked list
////////////////////////////////////////////////////////////////////////////////
void SIMC_List_Destroy(SIMC_LIST* list) {
	SIMC_LIST_ENTRY* entry;

	SIMC_SRW_EnterWrite(list->lock);
	entry = list->first;
	while (entry) {
		SIMC_LIST_ENTRY* _entry = entry;
		entry = entry->next;
		free(_entry);
	}
	SIMC_SRW_Destroy(list->lock);
	free(list);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Appends a new element.
///
/// With multithreading enabled several threads may add new elements at once,
/// but these operations will block reading threads from accessing the list.
///
/// The add operation requests write access and must be used sparingly.
///
/// @param[in] list Pointer to the linked list
/// @param[in] data Data stored in the new entry
////////////////////////////////////////////////////////////////////////////////
SIMC_LIST_ENTRY* SIMC_List_Append(SIMC_LIST* list, void* data) {
	SIMC_LIST_ENTRY* entry;

	//Start atomic write operation on list and block everyones access to it
	SIMC_SRW_EnterWrite(list->lock);

	//Create new entry
	entry = (SIMC_LIST_ENTRY*)malloc(sizeof(SIMC_LIST_ENTRY));
	entry->previous = list->last; //"last" will not change during atomic operation
	entry->next = 0;
	entry->data = data;

	//Fix pointers in list
	if (list->last) list->last->next = entry;
	list->last = entry;
	if (!list->first) list->first = entry;

	//End atomic operation on list and give everyone access
	SIMC_SRW_LeaveWrite(list->lock);
	return entry;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Removes an element, must be called from inside an iterator.
///
/// Only a single thread may attempt removing elements at any given time. When two
/// threads execute a simultaneous remove call, the list will enter undefined state.
///
/// The list removal can only be called from inside an iterator. Any element may be
/// requested from removal from the iterator, and the iterator will be finished
/// after the remove operation.
///
/// Here is the proper way to delete all elements by iterating through the list:
///
///		entry = SIMC_List_GetFirst(list);
///		while (entry) {
///			SIMC_List_Remove(list,entry); //Remove this entry
///			//entry = SIMC_List_GetNext(list,entry); //Wrong! Cannot iterate after remove operation!
///			entry = SIMC_List_GetFirst(variable->attributes); //Iteration must start again
///		}
///
/// The iterator must be terminated, because the list cannot be written to and read from
/// at the same time.
///
/// @param[in] list Pointer to the linked list
/// @param[in] entry List entry
////////////////////////////////////////////////////////////////////////////////
void SIMC_List_Remove(SIMC_LIST* list, SIMC_LIST_ENTRY* entry) {
#ifndef SIMC_SINGLETHREADED
	//Start atomic write operation on list and block everyones access to it
	SIMC_SRW_LeaveRead(list->lock);
	// <--- list/entry can be removed at this point, and the list will enter undefined state
	SIMC_SRW_EnterWrite(list->lock);
#endif

	//Fix pointers in entry neighbours
	if (entry->previous) entry->previous->next = entry->next;
	if (entry->next) entry->next->previous = entry->previous;

	//Fix pointers in linked list
	if (list->first == entry) list->first = entry->next;
	if (list->last == entry) list->last = entry->previous;

	//Destroy the entry data (which is why iterator must be terminated)
	free(entry);

	//End atomic operation on list and give everyone access
	SIMC_SRW_LeaveWrite(list->lock);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Moves one element in front of the other one.
///
/// This operation has similar limits as SIMC_List_Remove(), and must be called
/// from inside an iterator. The iterator must be restarted after executing this
/// operation.
///
/// @param[in] list Pointer to the linked list
/// @param[in] dest Source entry will be placed in front of dest entry
/// @param[in] source Entry to move
////////////////////////////////////////////////////////////////////////////////
void SIMC_List_MoveInFront(SIMC_LIST* list, SIMC_LIST_ENTRY* dest, SIMC_LIST_ENTRY* source) {
	if (dest == source) {
#ifndef SIMC_SINGLETHREADED
		SIMC_SRW_LeaveRead(list->lock);
#endif
		return;
	}

#ifndef SIMC_SINGLETHREADED
	//Start atomic operation on list and block everyones access to it
	SIMC_SRW_LeaveRead(list->lock);
	// <--- list may be modified at this point by another thread, and enter undefined state
	SIMC_SRW_EnterWrite(list->lock);
#endif

	//Work with pointers
	if (dest->previous)	dest->previous->next = dest->next; //Remove dest from chain
	if (dest->next) dest->next->previous = dest->previous;
	if (list->last == dest) list->last = dest->previous; //Remove from end
	if (list->first == dest) list->first = dest->next; //Remove from start

	if (source) { //Insert after source
		dest->next = source->next;
		if (source->next) source->next->previous = dest;
		source->next = dest;
		dest->previous = source;
		if (list->last == source) list->last = dest; //Can become last in list
	} else { //Insert in front
		if (list->first) list->first->previous = dest;
		dest->next = list->first;
		list->first = dest;
		if (!list->last) list->last = dest;
		dest->previous = 0;
	}

	//End atomic operation on list and give everyone access
	SIMC_SRW_LeaveWrite(list->lock);
}




////////////////////////////////////////////////////////////////////////////////
/// @brief Get first entry in the list and start iterating.
///
/// This operation will put list into "read" mode. Several threads may read from
/// the list at the same time.
///
/// Any threads attempting iterating over the list will be blocked when a write
/// is performed to the list (if list supports multithreading).
///
/// An example of iterating through the list:
///
///		entry = SIMC_List_GetFirst(list);
///		while (entry) {
///			EVDS_OBJECT* object = (EVDS_OBJECT*)SIMC_List_GetData(list,entry);
///			entry = SIMC_List_GetNext(list,entry);
///		}
///		SIMC_List_Stop(list,entry);
///
/// @returns Pointer to first entry in the list or a null pointer
/// @param[in] list Pointer to the linked list
////////////////////////////////////////////////////////////////////////////////
SIMC_LIST_ENTRY* SIMC_List_GetFirst(SIMC_LIST* list) {
#ifndef SIMC_SINGLETHREADED
	SIMC_SRW_EnterRead(list->lock); //Waits until list can be worked with
	if (!list->first) {
		SIMC_SRW_LeaveRead(list->lock);
		return 0;
	} else {
		return list->first;
	}
#else
	return list->first;
#endif
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Get last entry in the list and start iterating from the end.
///
/// See SICM_List_GetFirst(). This operation can be used to start iterating backwards
/// through the list.
///
/// @returns Pointer to last entry in the list or null
/// @param[in] list Pointer to the linked list
////////////////////////////////////////////////////////////////////////////////
SIMC_LIST_ENTRY* SIMC_List_GetLast(SIMC_LIST* list) {
#ifndef SIMC_SINGLETHREADED
	SIMC_SRW_EnterRead(list->lock); //Waits until list can be worked with
	if (!list->last) {
		SIMC_SRW_LeaveRead(list->lock);
		return 0;
	} else {
		return list->last;
	}
#else
	return list->last;
#endif
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Get next entry in list.
///
/// This operation will end the iterator when no next entry is present.
///
/// @returns Pointer to next entry in the list or null
/// @param[in] list Pointer to the linked list
/// @param[in] entry List entry
////////////////////////////////////////////////////////////////////////////////
//Requires access to list
SIMC_LIST_ENTRY* SIMC_List_GetNext(SIMC_LIST* list, SIMC_LIST_ENTRY* entry) {
#ifndef SIMC_SINGLETHREADED
	SIMC_LIST_ENTRY* next_entry = entry->next;
	if (!next_entry) SIMC_SRW_LeaveRead(list->lock);
	return next_entry;
#else
	return entry->next;
#endif
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Get previous entry in list.
///
/// This operation will end the iterator when no previous entry is present.
///
/// @returns Pointer to previous entry in the list or null
/// @param[in] list Pointer to the linked list
/// @param[in] entry List entry
////////////////////////////////////////////////////////////////////////////////
SIMC_LIST_ENTRY* SIMC_List_GetPrevious(SIMC_LIST* list, SIMC_LIST_ENTRY* entry) {
#ifndef SIMC_SINGLETHREADED
	SIMC_LIST_ENTRY* previous_entry = entry->previous;
	if (!previous_entry) SIMC_SRW_LeaveRead(list->lock);
	return previous_entry;
#else
	return entry->previous;
#endif
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Get data from the entry.
///
/// Entry must be valid. If a null pointer is passed, this function will crash.
///
/// @returns Returns pointer to data stored inside entry. Pointer must be casted to the correct
/// data structure type
/// @param[in] list Pointer to the linked list
/// @param[in] entry List entry
////////////////////////////////////////////////////////////////////////////////
void* SIMC_List_GetData(SIMC_LIST* list, SIMC_LIST_ENTRY* entry) {
	return entry->data;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Finishes the iterator.
///
/// For multithreading lists, stops iteration if entry is not null (assuming
/// SIMC_List_GetNext() was used to iterate, the iterator will be stopped automatically
/// when null entry is reached).
///
/// Must always be called if iterator was terminated earlier than entire list was traversed.
///
/// This operation is not required after the following operations:
///	- SIMC_List_Remove()
///	- SIMC_List_MoveInFront()
///
/// @param[in] list Pointer to the linked list
/// @param[in] entry List entry
////////////////////////////////////////////////////////////////////////////////
void SIMC_List_Stop(SIMC_LIST* list, SIMC_LIST_ENTRY* entry) {
#ifndef SIMC_SINGLETHREADED
	if (entry) SIMC_SRW_LeaveRead(list->lock);
#endif
}