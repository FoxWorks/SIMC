////////////////////////////////////////////////////////////////////////////////
/// @file
///
/// @brief Simulator Core Functions
////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2012-2015, Black Phoenix
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
#ifndef SIM_CORE_H
#define SIM_CORE_H
#ifdef __cplusplus
extern "C" {
#endif

// Required for size_t
#include <stdint.h>




////////////////////////////////////////////////////////////////////////////////
// Some macros and forward declarations
////////////////////////////////////////////////////////////////////////////////
#ifndef SIMC_DYNAMIC
#	define SIMC_API
#else
#	ifdef SIMC_LIBRARY
#		define SIMC_API __declspec(dllexport)
#	else
#		define SIMC_API __declspec(dllimport)
#	endif
#endif

//Forward structure declarations
typedef struct SIMC_LIST_ENTRY_TAG SIMC_LIST_ENTRY;
typedef struct SIMC_LIST_TAG SIMC_LIST;
typedef struct SIMC_STORAGEARRAY_TAG SIMC_STORAGEARRAY;
typedef struct SIMC_QUEUE_TAG SIMC_QUEUE;







////////////////////////////////////////////////////////////////////////////////
/// @defgroup SIMC Simulation Core
/// @brief Basic constants and callbacks
///
/// @{
////////////////////////////////////////////////////////////////////////////////

// Syntax error reading an XML file
typedef int SIMC_Callback_XMLSyntaxError(void* userdata, const char* error);
// Allocate memory
typedef void* SIMC_Callback_Allocate(void* userdata, size_t size);
// Free memory
typedef void SIMC_Callback_Free(void* userdata, void* pointer);

// No error
#define SIMC_OK								0
// Internal error
#define SIMC_ERROR_INTERNAL					1
// Error opening file (file not found or not accessible)
#define SIMC_ERROR_FILE						2
// Syntax error in configuration string/file
#define SIMC_ERROR_SYNTAX					3

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
/// @defgroup SIMC Simulation Core
/// @brief Basic set of utility functions shared between simulators
///
/// @{
////////////////////////////////////////////////////////////////////////////////

/// Thread handle
typedef void* SIMC_THREAD_ID;
/// Library handle
typedef void* SIMC_LIBRARY_ID;
/// Lock (critical section/mutex) handle
typedef void* SIMC_LOCK_ID;
/// Event handle
typedef void* SIMC_EVENT_ID;
/// SRW lock (slim read/write) handle
typedef void* SIMC_SRW_ID;
/// Invalid handle for use with SIMC_LOCK_ID, SIMC_SRW_ID, SIMC_THREAD_ID
#ifdef PLATFORM64
#	define SIMC_THREAD_BAD_ID ((void*)0xFFFFFFFF)
#else
#	define SIMC_THREAD_BAD_ID ((void*)0xFFFFFFFFFFFFFFFF)
#endif



// Set allocation functions (required if SIMC is to be used from DLL)
void SIMC_SetCallbacks(void* userdata, SIMC_Callback_Allocate* OnAllocate, SIMC_Callback_Free* OnFree);




// Warning: in a multi-threaded environment always call "SIMC_List_Stop" after iterating!
//  This will prevent stalling (iterating through list may lock its elements against removing).
//  Next/previous functions unlock the previous entry (so if you are iterating through entire
//   list, then there is no need for "Stop"... it will accept null pointer though).
// Get first list entry (starts iterating)
SIMC_API SIMC_LIST_ENTRY* SIMC_List_GetFirst(SIMC_LIST* list);
// Get last list entry (starts iterating)
SIMC_API SIMC_LIST_ENTRY* SIMC_List_GetLast(SIMC_LIST* list);
// Stop iterating
SIMC_API void SIMC_List_Stop(SIMC_LIST* list, SIMC_LIST_ENTRY* entry);
// Get next list entry (stops iterating if returns 0)
SIMC_API SIMC_LIST_ENTRY* SIMC_List_GetNext(SIMC_LIST* list, SIMC_LIST_ENTRY* entry);
// Get previous list entry (stops iterating if returns 0)
SIMC_API SIMC_LIST_ENTRY* SIMC_List_GetPrevious(SIMC_LIST* list, SIMC_LIST_ENTRY* entry);
// Get data from entry
SIMC_API void* SIMC_List_GetData(SIMC_LIST* list, SIMC_LIST_ENTRY* entry);




// Write to queue (returns false when queue is full)
SIMC_API void SIMC_Queue_EnterWrite(SIMC_QUEUE* queue, void** p_value);
// Stop reading from queue
SIMC_API int SIMC_Queue_LeaveWrite(SIMC_QUEUE* queue);
// Read from queue (returns false when no value was read)
SIMC_API int SIMC_Queue_EnterRead(SIMC_QUEUE* queue, void** p_value);
// Stop reading from queue
SIMC_API void SIMC_Queue_LeaveRead(SIMC_QUEUE* queue);
// Peek into queue (returns false when no value was read)
SIMC_API int SIMC_Queue_Peek(SIMC_QUEUE* queue, void** p_value);
// Clear queue
SIMC_API void SIMC_Queue_Clear(SIMC_QUEUE* queue);
// Gets approximate information about queue state
SIMC_API void SIMC_Queue_State(SIMC_QUEUE* queue, int* free_slots, int* used_slots);




// Load a library
SIMC_API SIMC_LIBRARY_ID SIMC_Library_Load(char* library_name);
// Unload library
SIMC_API void SIMC_Library_Unload(SIMC_LIBRARY_ID library);
// Get pointer to a function
SIMC_API void* SIMC_Library_GetFunction(SIMC_LIBRARY_ID library, char* function_name);




// Get current time
SIMC_API double SIMC_Thread_GetTime();
// Get current MJD time
SIMC_API double SIMC_Thread_GetMJDTime();
// Wait specific amount of time
SIMC_API void SIMC_Thread_Sleep(double time);

#ifndef SIMC_SINGLETHREADED
// Create thread
//SIMC_API SIMC_THREAD_ID SIMC_Thread_Create(void* funcPtr, void* userData);
#define SIMC_Thread_Create(funcPtr, userData) SIMC_Thread_CreateWithName(funcPtr, userData, __FUNCTION__ " (" __FILE__ ")" ) 
// Create thread with a name (displays the thread name in the debugger, if such feature is available)
SIMC_API SIMC_THREAD_ID SIMC_Thread_CreateWithName(void* funcPtr, void* userData, char* funcName);
// Get ID of the currently excuted thread
SIMC_API SIMC_THREAD_ID SIMC_Thread_GetCurrentID();
// Get unique ID of the currently executed thread (much faster, returns OS handle/ID)
SIMC_API SIMC_THREAD_ID SIMC_Thread_GetUniqueID();
// Wait for thread to complete
SIMC_API int SIMC_Thread_WaitFor(SIMC_THREAD_ID ID);
// Stop and kill thread
SIMC_API void SIMC_Thread_Kill(SIMC_THREAD_ID ID);
// Get total number of processors
SIMC_API int SIMC_Thread_GetNumProcessors();

// Create new lock
SIMC_API SIMC_LOCK_ID SIMC_Lock_Create();
// Destroy lock
SIMC_API void SIMC_Lock_Destroy(SIMC_LOCK_ID lockID);
// Enter lock
SIMC_API SIMC_LOCK_ID SIMC_Lock_Enter(SIMC_LOCK_ID lockID);
// Try to enter lock
//SIMC_API int SIMC_Lock_TryEnter(SIMC_LOCK_ID lockID);
// Leave lock
SIMC_API void SIMC_Lock_Leave(SIMC_LOCK_ID lockID);
// Wait for lock to be left
SIMC_API void SIMC_Lock_WaitFor(SIMC_LOCK_ID lockID);

// Create a new event
SIMC_API SIMC_EVENT_ID SIMC_Event_Create(char* eventName);
// Fire event
SIMC_API void SIMC_Event_Fire(SIMC_EVENT_ID eventID);
// Reset event
SIMC_API void SIMC_Event_Reset(SIMC_EVENT_ID eventID);
// Wait for event to be fired
SIMC_API int SIMC_Event_WaitFor(SIMC_EVENT_ID eventID, double time);

// Create new slim read/write lock
SIMC_API SIMC_SRW_ID SIMC_SRW_Create();
// Destroy SRW lock
SIMC_API void SIMC_SRW_Destroy(SIMC_SRW_ID srwID);
// Enter SRW lock for read operation
SIMC_API void SIMC_SRW_EnterRead(SIMC_SRW_ID srwID);
// Leave SRW lock (from read operation)
SIMC_API void SIMC_SRW_LeaveRead(SIMC_SRW_ID srwID);
// Enter SRW lock for write operation
SIMC_API void SIMC_SRW_EnterWrite(SIMC_SRW_ID srwID);
// Leave SRW lock (from write operation)
SIMC_API void SIMC_SRW_LeaveWrite(SIMC_SRW_ID srwID);
#else
//#define SIMC_Thread_Create()				((void)0)
#define SIMC_Thread_GetCurrentID(x)			((void)0)
#define SIMC_Thread_WaitFor(x)				((void)0)
#define SIMC_Thread_Kill(x)					((void)0)
#define SIMC_Thread_GetNumProcessors()		((void)0)

//#define SIMC_Lock_Create()				((void)0)
#define SIMC_Lock_Destroy(x)				((void)0)
#define SIMC_Lock_Enter(x)					((void)0)
#define SIMC_Lock_Leave(x)					((void)0)
#define SIMC_Lock_WaitFor(x)				((void)0)

//#define SIMC_SRW_Create()					((void)0)
#define SIMC_SRW_Destroy(x)					((void)0)
#define SIMC_SRW_EnterRead(x)				((void)0)
#define SIMC_SRW_LeaveRead(x)				((void)0)
#define SIMC_SRW_EnterWrite(x)				((void)0)
#define SIMC_SRW_LeaveWrite(x)				((void)0)
#endif
////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////


//If required, include internal functions
#ifdef SIMC_LIBRARY
#	include "sim_internal.h"
#endif


#ifdef __cplusplus
}
#endif
#endif
