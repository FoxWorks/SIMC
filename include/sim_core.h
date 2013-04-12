////////////////////////////////////////////////////////////////////////////////
/// @file
///
/// @brief Simulator Core Functions
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
#ifndef SIM_CORE_H
#define SIM_CORE_H
#ifdef __cplusplus
extern "C" {
#endif




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
typedef struct SIMC_QUEUE_TAG SIMC_QUEUE;




////////////////////////////////////////////////////////////////////////////////
/// @defgroup SIMC Simulation Core
/// @brief Basic set of utility functions shared between simulators
///
/// @{
////////////////////////////////////////////////////////////////////////////////

/// Thread handle
typedef void* SIMC_THREAD_ID;
/// Lock (critical section/mutex) handle
typedef void* SIMC_LOCK_ID;
/// SRW lock (slim read/write) handle
typedef void* SIMC_SRW_ID;
/// Invalid handle for use with SIMC_LOCK_ID, SIMC_SRW_ID, SIMC_THREAD_ID
#define SIMC_THREAD_BAD_ID ((void*)0xFFFFFFFF)




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




// Get current time
SIMC_API double SIMC_Thread_GetTime();
// Get current MJD time
SIMC_API double SIMC_Thread_GetMJDTime();
// Wait specific amount of time
SIMC_API void SIMC_Thread_Sleep(double time);

#ifndef SIMC_SINGLETHREADED
// Create thread
SIMC_API SIMC_THREAD_ID SIMC_Thread_Create(void* funcPtr, void* userData);
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
// Leave lock
SIMC_API void SIMC_Lock_Leave(SIMC_LOCK_ID lockID);
// Wait for lock to be left
SIMC_API void SIMC_Lock_WaitFor(SIMC_LOCK_ID lockID);

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