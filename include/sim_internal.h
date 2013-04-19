////////////////////////////////////////////////////////////////////////////////
/// @file
///
/// @brief Simulator Core Functions (internal)
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
#ifndef SIM_INTERNAL_H
#define SIM_INTERNAL_H
#ifdef __cplusplus
extern "C" {
#endif

// Syntax error reading an XML file
typedef int SIMC_Callback_XMLSyntaxError(void* userdata, const char* error);

// No error
#define SIMC_OK								0
// Internal error
#define SIMC_ERROR_INTERNAL					1
// Error opening file (file not found or not accessible)
#define SIMC_ERROR_FILE						2
// Syntax error in configuration string/file
#define SIMC_ERROR_SYNTAX					3

/// Compatibility with Windows systems
#ifdef _WIN32
#define snprintf _snprintf
#define snscanf _snscanf
#define alloca _alloca
#endif




////////////////////////////////////////////////////////////////////////////////
/// @ingroup SIMC_UTILS
/// @struct SIMC_LIST_ENTRY
/// @brief A single entry inside the list
///
/// A single entry of the SIMC_LIST. Use SIMC_List_GetData() to get pointer to data
/// from the entry, and SIMC_List_GetNext() to iterate forward through the list.
////////////////////////////////////////////////////////////////////////////////
#ifndef DOXYGEN_INTERNAL_STRUCTS
struct SIMC_LIST_ENTRY_TAG {
	struct SIMC_LIST_ENTRY_TAG* next;			//Next entry
	struct SIMC_LIST_ENTRY_TAG* previous;		//Previous entry
	void* data;									//Data associated with this entry
};
#endif




////////////////////////////////////////////////////////////////////////////////
/// @ingroup SIMC_UTILS
/// @struct SIMC_LIST
/// @brief List of pointers
///
/// Internally implemented as a linked list. It is thread-safe, and allows fast
/// read operations (but slow write operations).
///
/// SIMC_LIST is returned by many functions from the simulator API's. Here's a simple example
/// of iterating through the list of objects in EVDS:
///
///		SIMC_LIST_ENTRY* entry;
///		SIMC_LIST* list;
///		
///		EVDS_Object_GetChildren(object,&list);
///		entry = SIMC_List_GetFirst(list);
///		while (entry) {
///			EVDS_OBJECT* object = (EVDS_OBJECT*)SIMC_List_GetData(list,entry);
///		
///			//..code..
///		
///			//Move to next object in list
///			entry = SIMC_List_GetNext(list,entry);
///		}
///		SIMC_List_Stop(list,entry);
///
/// The last call to SIMC_List_Stop() is only required when terminating iteration early.
/// It is not really required for iterating through entire list of objects.
////////////////////////////////////////////////////////////////////////////////
#ifndef DOXYGEN_INTERNAL_STRUCTS
struct SIMC_LIST_TAG {
#ifndef SIMC_SINGLETHREADED
	SIMC_SRW_ID lock;				//Lock for writing/changing list
#endif
	SIMC_LIST_ENTRY* first;			//First entry
	SIMC_LIST_ENTRY* last;			//Last entry
};
#endif




////////////////////////////////////////////////////////////////////////////////
/// @ingroup SIMC_UTILS
/// @struct SIMC_QUEUE
/// @brief 64-bit queue between threads
///
/// Only one thread may write to this queue, only one thread may read from
/// this queue.
////////////////////////////////////////////////////////////////////////////////
#ifndef DOXYGEN_INTERNAL_STRUCTS
struct SIMC_QUEUE_TAG {
/*#ifndef SIMC_SINGLETHREADED
	unsigned int concurrent_w;	//Queue supports concurrent writes
	unsigned int concurrent_r;	//Queue supports concurrent reads
	SIMC_LOCK_ID lock_w;		//Lock for writing
	SIMC_LOCK_ID lock_r;		//Lock for reading
#endif*/

	void* data;			//Pointer to buffer data
	void* data_last;	//Last valid value in buffer
	int size;			//Number of entries in buffer
	int element_size;	//Size of one entry

	void* write_ptr;	//Current buffer write pointer
	void* read_ptr;		//Current buffer read pointer
};
#endif




////////////////////////////////////////////////////////////////////////////////
// Internal API
////////////////////////////////////////////////////////////////////////////////
#ifndef SIMC_SINGLETHREADED
// Initialize threading (reference-counted, must be paired with a deinitializer)
void SIMC_Thread_Initialize();
// Deinitialize threading (free resources)
void SIMC_Thread_Deinitialize();
#endif

// Create new queue
void SIMC_Queue_Create(SIMC_QUEUE** p_queue, int size, int element_size);
// Destroy queue
void SIMC_Queue_Destroy(SIMC_QUEUE* queue);

// Create new linked list
void SIMC_List_Create(SIMC_LIST** p_list, int multithreaded);
// Destroy linked list (must not be used by any threads - locking not checked)
void SIMC_List_Destroy(SIMC_LIST* list);
// Moves element src in front of element dest
void SIMC_List_MoveInFront(SIMC_LIST* list, SIMC_LIST_ENTRY* dest, SIMC_LIST_ENTRY* source);

// Append data to the list (very slow and halts every other thread)
SIMC_LIST_ENTRY* SIMC_List_Append(SIMC_LIST* list, void* data);
// Remove data from the list (very slow and halts every other thread, call only inside iterator)
void SIMC_List_Remove(SIMC_LIST* list, SIMC_LIST_ENTRY* entry);

#ifndef SIMC_SINGLETHREADED
void SIMC_List_EnterRead(SIMC_LIST* list);
void SIMC_List_LeaveRead(SIMC_LIST* list);
void SIMC_List_EnterWrite(SIMC_LIST* list);
void SIMC_List_LeaveWrite(SIMC_LIST* list);
#endif


#ifdef __cplusplus
}
#endif
#endif
