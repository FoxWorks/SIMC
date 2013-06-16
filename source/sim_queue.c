////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2012-2013, Black Phoenix
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the GNU Lesser General Public License as published by the Free Software
/// Foundation; either version 2 of the License, or (at your option) any later
/// version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
/// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
/// details.
///
/// You should have received a copy of the GNU Lesser General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place - Suite 330, Boston, MA  02111-1307, USA.
///
/// Further information about the GNU Lesser General Public License can also be found on
/// the world wide web at http://www.gnu.org.
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include "sim_core.h"


////////////////////////////////////////////////////////////////////////////////
/// @brief Create a new threading-safe queue of a fixed size
////////////////////////////////////////////////////////////////////////////////
void SIMC_Queue_Create(SIMC_QUEUE** p_queue, int size, int element_size) {
	SIMC_QUEUE* queue = (SIMC_QUEUE*)malloc(sizeof(SIMC_QUEUE));

	queue->data = malloc(element_size*size);
	queue->data_last = (void*)((char*)queue->data + element_size*(size - 1));
	queue->write_ptr = queue->data;
	queue->read_ptr = queue->data;
	queue->size = size;
	queue->element_size = element_size;

	*p_queue = queue;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Destroy queue
////////////////////////////////////////////////////////////////////////////////
void SIMC_Queue_Destroy(SIMC_QUEUE* queue) {
	free(queue->data);
	free(queue);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Locks queue against anyone trying to write again.
////////////////////////////////////////////////////////////////////////////////
void SIMC_Queue_EnterWrite(SIMC_QUEUE* queue, void** p_value) {
	*p_value = queue->write_ptr;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Unlocks queue from write access.
////////////////////////////////////////////////////////////////////////////////
int SIMC_Queue_LeaveWrite(SIMC_QUEUE* queue) {
	void* new_ptr;

	//Create new pointer
	if (queue->write_ptr == queue->data_last) {
		new_ptr = queue->data;
	} else {
		new_ptr = (char*)queue->write_ptr + queue->element_size;
	}

	//If possible, move queue pointer up
	if (new_ptr != queue->read_ptr) {
		queue->write_ptr = new_ptr;
		return 1;
	} else {
		//Unlock
		return 0;
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Prevents read pointer from moving until reading from it was finished.
////////////////////////////////////////////////////////////////////////////////
int SIMC_Queue_EnterRead(SIMC_QUEUE* queue, void** p_value) {
	if (p_value) *p_value = queue->read_ptr;

	//If possible, move pointer up
	if (queue->read_ptr != queue->write_ptr) {
		if (!p_value) SIMC_Queue_LeaveRead(queue);
		return 1;
	} else {
		//Unlock, queue full
		return 0;
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Leaves reading mode.
////////////////////////////////////////////////////////////////////////////////
void SIMC_Queue_LeaveRead(SIMC_QUEUE* queue) {
	if (queue->read_ptr == queue->data_last) {
		queue->read_ptr = queue->data;
	} else {
#ifdef _WIN32
		(char*)queue->read_ptr += queue->element_size; //FIXME: sort this out between compilers
#else
		queue->read_ptr += queue->element_size;
#endif
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Peek into queue (returns false when no value was read)
////////////////////////////////////////////////////////////////////////////////
int SIMC_Queue_Peek(SIMC_QUEUE* queue, void** p_value) {
	//Lock
	*p_value = queue->read_ptr;
	if (queue->read_ptr != queue->write_ptr) {
		return 1;
	} else {
		//Unlock
		return 0;
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Clear queue.
////////////////////////////////////////////////////////////////////////////////
void SIMC_Queue_Clear(SIMC_QUEUE* queue) {
	queue->write_ptr = queue->data;
	queue->read_ptr = queue->data;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Gets approximate information about queue state.
////////////////////////////////////////////////////////////////////////////////
void SIMC_Queue_State(SIMC_QUEUE* queue, int* free_slots, int* used_slots) {
	char* old_read = queue->read_ptr;
	char* old_write = queue->write_ptr;
	int used_slots_;

	if (old_read <= old_write) {
		used_slots_ = (int)(old_write-old_read)/queue->element_size;
	} else {
		used_slots_ = (int)(queue->size-(old_write-old_read))/queue->element_size;
		if (old_read == old_write) used_slots_ = 0;
	}

	if (free_slots) *free_slots = queue->size - used_slots_;
	if (used_slots) *used_slots = used_slots_;
}
