////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
/// Portions of this source file are derived from GLFW (http://www.glfw.org).
/// The original work belongs to its authors:
///  Copyright (C) 2002-2006, Marcus Geelnard
///  Copyright (C) 2006-2011, Camilla Berglund
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
#include "sim_core.h"
#ifdef _WIN32
#  include "windows.h"
#else
#  include <stdlib.h>
#  include <pthread.h>
#  include <sched.h>
#  include <signal.h>
#  include <sys/time.h>
#  include <unistd.h>
#endif

// Use WinAPI/POSIX implementation of SRW locks instead of custom implementation (windows-only)
#define SIMC_NATIVE_SRW


////////////////////////////////////////////////////////////////////////////////
// Internal data structures and typedefs
////////////////////////////////////////////////////////////////////////////////
#ifndef SIMC_SINGLETHREADED

/// Callback for the function that can be used as a pointer for a thread
typedef void SIMC_THREAD_FUNCTION(void* userdata);

#ifndef DOXYGEN_INTERNAL_STRUCTS
typedef struct SIMC_THREAD_TAG {
	//Shared members
	struct SIMC_THREAD_TAG	*previous, *next;  //Linked list implementation
	SIMC_THREAD_ID			id;               //Thread ID
	void*					function;         //Function to call

	//System-specific handles
#ifdef _WIN32
	HANDLE         handle;
	DWORD          winID;
#else
	pthread_t      posixID;
#endif
} SIMC_THREAD;
#endif


//Global variables for threading system
SIMC_THREAD_ID SIMC_Thread_NextID = (void*)1;
SIMC_THREAD SIMC_Thread_Main;
unsigned int SIMC_Instance_Counter = 0;


//Get thread pointer
SIMC_THREAD* SIMC_Thread_Internal_GetPointer(SIMC_THREAD_ID ID) {
	SIMC_THREAD* thread;
	for (thread = &SIMC_Thread_Main; thread != NULL; thread = thread->next) {
		if (thread->id == ID) break;
	}
	return thread;
}

//Remove thread
void SIMC_Thread_Internal_Remove(SIMC_THREAD* thread) {
	if (thread->previous != NULL) thread->previous->next = thread->next;
	if (thread->next != NULL) thread->next->previous = thread->previous;
	free((void*)thread);
}
#endif








////////////////////////////////////////////////////////////////////////////////
// Windows specific code
////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#ifndef SIMC_SINGLETHREADED

CRITICAL_SECTION SIMC_Thread_CriticalSection;
void SIMC_Thread_EnterCriticalSection() {
	EnterCriticalSection(&SIMC_Thread_CriticalSection);
}

void SIMC_Thread_LeaveCriticalSection() {
	LeaveCriticalSection(&SIMC_Thread_CriticalSection);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Wrapper function for the newly created threads
////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI SIMC_Thread_Internal_New(LPVOID lpParam) {
	SIMC_THREAD_FUNCTION* threadfun;
	SIMC_THREAD* thread;

	//Get pointer to thread information for current thread
	thread = SIMC_Thread_Internal_GetPointer(SIMC_Thread_GetCurrentID());
	if (thread == NULL) return 0;

	//Get user thread function pointer
	threadfun = thread->function;

	//Call the user thread function
	threadfun((void*)lpParam);

	//Remove thread from thread list
	SIMC_Thread_EnterCriticalSection();
	SIMC_Thread_Internal_Remove(thread);
	SIMC_Thread_LeaveCriticalSection();

	//Return and kill the thread
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Create a new thread.
///
/// Creates new thread and starts it immediately. For example:
/// ~~~{.c}
///		void test_thread(void* data) {
///		}
///
///		SIMC_Thread_Create(test_thread,data);
/// ~~~
///
/// @param[in] func_ptr Pointer to the thread function
/// @param[in] userdata Variable to pass into the threading function
///
/// @returns Thread handle
////////////////////////////////////////////////////////////////////////////////
SIMC_THREAD_ID SIMC_Thread_Create(void* func_ptr, void* userdata) {
	SIMC_THREAD_ID ID;
	SIMC_THREAD* thread, *thread_tmp;
	HANDLE hThread;
	DWORD dwThreadId;

	//Enter critical section
	SIMC_Thread_EnterCriticalSection();

	//Create a new thread information memory area
	thread = (SIMC_THREAD*)malloc(sizeof(SIMC_THREAD));
	if (thread == NULL) {
		SIMC_Thread_LeaveCriticalSection();
		return SIMC_THREAD_BAD_ID;
	}

	//Get a new unique thread id
	ID = (((char*)SIMC_Thread_NextID)++);

	//Store thread information in the thread list
	thread->function = func_ptr;
	thread->id       = ID;

	//Create thread
	hThread = CreateThread(
	              NULL,              //Default security attributes
	              0,                 //Default stack size (1 MB)
	              SIMC_Thread_Internal_New,       //Thread function (a wrapper function)
	              (LPVOID)userdata,  //Argument to thread is the user argument
	              0,                 //Default creation flags
	              &dwThreadId        //Returned thread identifier
	         );

	//Did the thread creation fail?
	if (hThread == NULL) {
		free((void*)thread);
		SIMC_Thread_LeaveCriticalSection();
		return SIMC_THREAD_BAD_ID;
	}

	//Store more thread information in the thread list
	thread->handle = hThread;
	thread->winID = dwThreadId;

	//Append thread to thread list
	thread_tmp = &SIMC_Thread_Main;
	while (thread_tmp->next != NULL) {
		thread_tmp = thread_tmp->next;
	}
	thread_tmp->next = thread;
	thread->previous = thread_tmp;
	thread->next     = NULL;

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();

	//Return the thread ID
	//log_writem("Spawned thread [%p] (SIMC_THREAD_ID[0x%x])",funcPtr,ID);
	return ID;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Get handle of the currently executed thread.
///
/// This function returns the handle of a currently executed thread. It's a relatively
/// slow call, and should not be used in tight loops.
///
/// For checking and identifying threads in a fast way, use SIMC_Thread_GetUniqueID() API
/// call. The handles returned by both calls are not compatible.
///
/// @returns Thread handle
////////////////////////////////////////////////////////////////////////////////
SIMC_THREAD_ID SIMC_Thread_GetCurrentID() {
	SIMC_THREAD* thread;
	SIMC_THREAD_ID ID = SIMC_THREAD_BAD_ID;
	DWORD WinID;

	//Get Windows thread ID
	WinID = GetCurrentThreadId();

	//Enter critical section (to avoid an inconsistent thread list)
	SIMC_Thread_EnterCriticalSection();

	//Loop through entire list of threads to find the matching Windows thread ID
	for (thread = &SIMC_Thread_Main; thread != NULL; thread = thread->next) {
		if (thread->winID == WinID) {
			ID = thread->id;
			break;
		}
	}

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();
	return ID;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Return unique identifier of the current thread.
///
/// Returns OS handle for the currently executed thread. Unlike SIMC_Thread_GetCurrentID()
/// does not carry any significant overhead. This call is mostly used to identify threads:
/// ~~~{.c}
///		thread_id = SIMC_Thread_GetUniqueID()
///		...
///		if (SIMC_Thread_GetUniqueID() == thread_id) ...
/// ~~~
///
/// @returns Unique handle
////////////////////////////////////////////////////////////////////////////////
SIMC_THREAD_ID SIMC_Thread_GetUniqueID() {
	return (SIMC_THREAD_ID)GetCurrentThreadId();
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Waits current thread until the target thread finishes execution.
///
/// Several of these calls can be used together to wait until an array of worker
/// threads complete. For example:
/// ~~~{.c}
///		SIMC_THREAD_ID workers[32];
///		for (i = 0; i < 32; i++) workers[i] = SIMC_Thread_Create(worker_thread,(void*)i);
///		for (i = 0; i < 32; i++) SIMC_Thread_WaitFor(workers[i]);
/// ~~~
///
/// @param[in] ID Thread handle
////////////////////////////////////////////////////////////////////////////////
int SIMC_Thread_WaitFor(SIMC_THREAD_ID ID) {
	DWORD result;
	HANDLE hThread;
	SIMC_THREAD* thread;

	//Enter critical section
	SIMC_Thread_EnterCriticalSection();

	//Get thread information pointer
	thread = SIMC_Thread_Internal_GetPointer(ID);

	//Is the thread already dead?
	if (thread == NULL)	{
		SIMC_Thread_LeaveCriticalSection();
		return 1;
	}

	//Get thread handle
	hThread = thread->handle;

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();

	//Wait for thread to die
	//if(waitmode == GLFW_WAIT) {
	result = WaitForSingleObject(hThread, INFINITE);
	//} else if(waitmode == GLFW_NOWAIT) {
	//result = WaitForSingleObject(hThread, 0);
	//} else {
	//return 0;
	//}

	//Did we have a time-out?
	if (result == WAIT_TIMEOUT) return 0;
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Stop and remove thread by handle.
///
/// @param[in] ID Thread ID
////////////////////////////////////////////////////////////////////////////////
void SIMC_Thread_Kill(SIMC_THREAD_ID ID) {
	SIMC_THREAD* thread;

	//Enter critical section
	SIMC_Thread_EnterCriticalSection();

	//Get thread information pointer
	thread = SIMC_Thread_Internal_GetPointer(ID);
	if (thread == NULL) {
		SIMC_Thread_LeaveCriticalSection();
		return;
	}

	//Kill the process
	if (TerminateThread(thread->handle, 0)) {
		//Close thread handle
		CloseHandle(thread->handle);

		//Remove thread from thread list
		SIMC_Thread_Internal_Remove(thread);
	}

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();
}
#endif


////////////////////////////////////////////////////////////////////////////////
/// @brief Wait current thread for the specified time.
///
/// This function accepts time in seconds. On all platforms wait time cannot
/// exceed \f$2^{31}\f$ seconds, and cannot be less than one millisecond.
///
/// Passing wait time of 0.0 will simply yield execution of the current thread.
///
/// @param[in] time Delay in seconds, or 0.0 if thread must yield execution
////////////////////////////////////////////////////////////////////////////////
void SIMC_Thread_Sleep(double time) {
	DWORD t;

	if (time == 0.0) {
		SwitchToThread();
	}

	if (time <= 1e-3) {
		t = 1;
	} else if (time > 2147483647.0) {
		t = 2147483647;
	} else {
		t = (DWORD)(time*1000.0 + 0.5);
	}
	Sleep(t);
}


#ifndef SIMC_SINGLETHREADED
////////////////////////////////////////////////////////////////////////////////
/// @brief Thread get number of processors (cores).
///
/// This returns number of processors as reported by the OS - returns double the
/// physical amount on hyperthreaded systems.
///
/// @returns Number of processors (cores) on the system
////////////////////////////////////////////////////////////////////////////////
int SIMC_Thread_GetNumProcessors() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return (int)si.dwNumberOfProcessors;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Creates an exclusive lock (mutex/critical section).
/// 
/// Example of use:
/// ~~~{.c}
///		SIMC_LOCK_ID lock = SIMC_Lock_Create();
///		SIMC_Lock_Enter(lock);
///			...
///		SIMC_Lock_Leave(lock);
/// ~~~
///
/// @returns Lock handle
////////////////////////////////////////////////////////////////////////////////
SIMC_LOCK_ID SIMC_Lock_Create() {
	CRITICAL_SECTION* lock;

	//Allocate memory for lock
	lock = (CRITICAL_SECTION*)malloc(sizeof(SIMC_Thread_CriticalSection));
	if (!lock)
	{
		return SIMC_THREAD_BAD_ID;
	}
	if ((SIMC_LOCK_ID)lock == SIMC_THREAD_BAD_ID)
	{
		return SIMC_THREAD_BAD_ID;
	}

	//Initialize lock
	InitializeCriticalSection(lock);

	//Cast to lock ID and return
	return (SIMC_LOCK_ID)lock;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Deletes an exclusive lock.
/// @param[in] lockID Lock handle
////////////////////////////////////////////////////////////////////////////////
void SIMC_Lock_Destroy(SIMC_LOCK_ID lockID) {
	DeleteCriticalSection((CRITICAL_SECTION*)lockID);
	free((void*)lockID);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Enter exclusive lock.
/// @param[in] lockID Lock handle
/// @returns Lock handle passed into the function
////////////////////////////////////////////////////////////////////////////////
SIMC_LOCK_ID SIMC_Lock_Enter(SIMC_LOCK_ID lockID) {
	if (lockID != SIMC_THREAD_BAD_ID) {
		//Wait for lock to be released
		EnterCriticalSection((CRITICAL_SECTION*)lockID);
	}
	return lockID;
}

/*int SIMC_Lock_TryEnter(SIMC_LOCK_ID lockID) {
	if (lockID != SIMC_THREAD_BAD_ID) {
		return TryEnterCriticalSection((CRITICAL_SECTION*)lockID) != 0;
	}
	return 0;
}*/


////////////////////////////////////////////////////////////////////////////////
/// @brief Leave exclusive lock.
/// @param[in] lockID Lock handle
////////////////////////////////////////////////////////////////////////////////
void SIMC_Lock_Leave(SIMC_LOCK_ID lockID) {
	if (lockID != SIMC_THREAD_BAD_ID) {
		LeaveCriticalSection((CRITICAL_SECTION*)lockID);
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Waits until exclusive lock is left by locking thread.
///
/// It is useful to make a lock when some worker thread is running, and then simply
/// unlock it in the end. Main thread can use SIMC_Lock_WaitFor() to wait until
/// the worked thread(s) complete it task.
///
/// @param[in] lockID Lock handle
////////////////////////////////////////////////////////////////////////////////
void SIMC_Lock_WaitFor(SIMC_LOCK_ID lockID) {
	SIMC_Lock_Leave(SIMC_Lock_Enter(lockID));
}


//Slim read-write locks (WinAPI implementation)
#ifdef SIMC_NATIVE_SRW

////////////////////////////////////////////////////////////////////////////////
/// @brief Create slim read/write lock.
///
/// SRW locks are useful for protecting data when write operations are much more
/// sparse than read operations. For example:
/// ~~~{.c}
///		SIMC_SRW_ID lock = SIMC_SRW_Create();
///		SIMC_SRW_EnterRead(lock);
///			... data will not be changed here ...
///		SIMC_SRW_LeaveRead(lock);
///
///		SIMC_SRW_EnterWrite(lock);
///			... no other thread will read data ...
///		SIMC_SRW_LeaveWrite(lock);
/// ~~~
///
/// @returns Lock handle
////////////////////////////////////////////////////////////////////////////////
SIMC_SRW_ID SIMC_SRW_Create() {
	PSRWLOCK lock = (PSRWLOCK)malloc(sizeof(SRWLOCK));
	InitializeSRWLock(lock);
	return (SIMC_SRW_ID)lock;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Destroy SRW lock.
/// @param[in] srwID Lock handle
////////////////////////////////////////////////////////////////////////////////
void SIMC_SRW_Destroy(SIMC_SRW_ID srwID) {
	PSRWLOCK lock = (PSRWLOCK)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;
	free(lock);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Enter shared reading section (within this section, no writes will be performed to data).
/// @param[in] srwID Lock handle
////////////////////////////////////////////////////////////////////////////////
void SIMC_SRW_EnterRead(SIMC_SRW_ID srwID) {
	PSRWLOCK lock = (PSRWLOCK)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;
	AcquireSRWLockShared(lock);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Leave shared reading section.
/// @param[in] srwID Lock handle
////////////////////////////////////////////////////////////////////////////////
void SIMC_SRW_LeaveRead(SIMC_SRW_ID srwID) {
	PSRWLOCK lock = (PSRWLOCK)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;
	ReleaseSRWLockShared(lock);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Enter exclusive writing section.
/// @param[in] srwID Lock handle
////////////////////////////////////////////////////////////////////////////////
void SIMC_SRW_EnterWrite(SIMC_SRW_ID srwID) {
	PSRWLOCK lock = (PSRWLOCK)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	AcquireSRWLockExclusive(lock);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Leave exclusive writing section.
/// @param[in] srwID Lock handle
////////////////////////////////////////////////////////////////////////////////
void SIMC_SRW_LeaveWrite(SIMC_SRW_ID srwID) {
	PSRWLOCK lock = (PSRWLOCK)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;
	ReleaseSRWLockExclusive(lock);
}

#else

#define SIMC_SRW_THRESHOLD	0xFFFF
typedef struct SIMC_SRW_LOCK_TAG {
	SIMC_LOCK_ID write_lock;
	long srw_lock;
} SIMC_SRW_LOCK;

SIMC_SRW_ID SIMC_SRW_Create() {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)malloc(sizeof(SIMC_SRW_LOCK));
	lock->write_lock = SIMC_Lock_Create();
	lock->srw_lock = 0;
	return (SIMC_SRW_ID)lock;
}

void SIMC_SRW_Destroy(SIMC_SRW_ID srwID) {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	SIMC_Lock_Destroy(lock->write_lock);
	free(lock);
}

void SIMC_SRW_EnterRead(SIMC_SRW_ID srwID) {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	while (InterlockedIncrement(&lock->srw_lock) < 0) {
        InterlockedDecrement(&lock->srw_lock);
        SwitchToThread();
    }
}

void SIMC_SRW_LeaveRead(SIMC_SRW_ID srwID) {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	InterlockedDecrement(&lock->srw_lock);
}

void SIMC_SRW_EnterWrite(SIMC_SRW_ID srwID) {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	SIMC_Lock_Enter(lock->write_lock); //Block other threads from writing
	InterlockedExchangeAdd(&lock->srw_lock,-SIMC_SRW_THRESHOLD);
	while ((lock->srw_lock) > -SIMC_SRW_THRESHOLD) { //Wait until read threads finish
        SwitchToThread();
    }
}

void SIMC_SRW_LeaveWrite(SIMC_SRW_ID srwID) {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	while ((lock->srw_lock) != -SIMC_SRW_THRESHOLD) { //Block execution if not leaving write lock
		SwitchToThread();
	}
	InterlockedExchangeAdd(&lock->srw_lock,SIMC_SRW_THRESHOLD);
	SIMC_Lock_Leave(lock->write_lock);
}

#endif



////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize threading
////////////////////////////////////////////////////////////////////////////////
void SIMC_Thread_Initialize() {
	if (SIMC_Instance_Counter == 0) {
		//Initialize critical section handle
		InitializeCriticalSection(&SIMC_Thread_CriticalSection);
	
		//The first thread (the main thread) has ID 0
		SIMC_Thread_NextID = 0;
	
		//Fill out information about the main thread (this thread)
		SIMC_Thread_Main.id       = (((char*)SIMC_Thread_NextID)++);
		SIMC_Thread_Main.function = 0;
		SIMC_Thread_Main.handle   = GetCurrentThread();
		SIMC_Thread_Main.winID    = GetCurrentThreadId();
		SIMC_Thread_Main.previous = 0;
		SIMC_Thread_Main.next     = 0;
	}

	SIMC_Instance_Counter++;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Deinitialize threading and shut down all threads
////////////////////////////////////////////////////////////////////////////////
void SIMC_Thread_Deinitialize() {
	SIMC_THREAD* thread, *thread_next;

	//Check reference counter
	SIMC_Instance_Counter--;
	if (SIMC_Instance_Counter > 0) return;

	//Enter critical section
	SIMC_Thread_EnterCriticalSection();

	//Kill all threads
	thread = SIMC_Thread_Main.next;
	while (thread != NULL) {
		//Get pointer to next thread
		thread_next = thread->next;

		//Simply murder the process, no mercy!
		if (TerminateThread(thread->handle, 0)) {
			//Close thread handle
			CloseHandle(thread->handle);

			//Free memory allocated for this thread
			free((void*)thread);
		}

		//Select next thread in list
		thread = thread_next;
	}

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();

	//Delete critical section handle
	DeleteCriticalSection(&SIMC_Thread_CriticalSection);
}
#endif








#else
////////////////////////////////////////////////////////////////////////////////
// POSIX specific code
////////////////////////////////////////////////////////////////////////////////
#ifndef SIMC_SINGLETHREADED

pthread_mutex_t SIMC_Thread_CriticalSection;

void SIMC_Thread_EnterCriticalSection() {
	pthread_mutex_lock(&SIMC_Thread_CriticalSection);
}

void SIMC_Thread_LeaveCriticalSection() {
	pthread_mutex_unlock(&SIMC_Thread_CriticalSection);
}


void* SIMC_Thread_Internal_New(void *arg) {
	SIMC_THREAD_FUNCTION* threadfunc;
	SIMC_THREAD *thread;
	pthread_t posixID;

	//Get current thread ID
	posixID = pthread_self();

	//Enter critical section
	SIMC_Thread_EnterCriticalSection();

	//Loop through entire list of threads to find the matching POSIX thread ID
	for (thread = &SIMC_Thread_Main; thread != NULL; thread = thread->next) {
		if(thread->posixID == posixID) break;
	}

	if (thread == NULL) {
		SIMC_Thread_LeaveCriticalSection();
		return NULL;
	}

	//Get user thread function pointer
	threadfunc = thread->function;

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();

	//Call the user thread function
	threadfunc(arg);

	//Remove thread from thread list
	SIMC_Thread_EnterCriticalSection();
	SIMC_Thread_Internal_Remove(thread);
	SIMC_Thread_LeaveCriticalSection();

	//When the thread function returns, the thread will die...
	return NULL;
}


SIMC_THREAD_ID SIMC_Thread_Create(void* funcPtr, void* userData) {
	SIMC_THREAD_ID ID;
	SIMC_THREAD *thread, *thread_tmp;
	int result;

	//Enter critical section
	SIMC_Thread_EnterCriticalSection();

	//Create a new thread information memory area
	thread = (SIMC_THREAD*)malloc(sizeof(SIMC_THREAD));
	if (thread == NULL) {
		SIMC_Thread_LeaveCriticalSection();
		return SIMC_THREAD_BAD_ID;
	}

	//Get a new unique thread id
	ID = SIMC_Thread_NextID++;

	//Store thread information in the thread list
	thread->function = funcPtr;
	thread->id       = ID;

	//Create thread
	result = pthread_create(
		&thread->posixID,      //Thread handle
		NULL,             //Default thread attributes
		SIMC_Thread_Internal_New,      //Thread function (a wrapper function)
		(void*)userData   //Argument to thread is user argument
   	);

	//Did the thread creation fail?
	if (result != 0) {
		free((void*)thread);
		SIMC_Thread_LeaveCriticalSection();
		return SIMC_THREAD_BAD_ID;
	}

	//Append thread to thread list
	thread_tmp = &SIMC_Thread_Main;
	while (thread_tmp->next != NULL) {
		thread_tmp = thread_tmp->next;
	}
	thread_tmp->next = thread;
	thread->previous = thread_tmp;
	thread->next     = NULL;

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();
	return ID;
}


SIMC_THREAD_ID SIMC_Thread_GetCurrentID() {
	SIMC_THREAD *thread;
	SIMC_THREAD_ID ID = SIMC_THREAD_BAD_ID;
	pthread_t posixID;

	//Get current thread ID
	posixID = pthread_self();

	//Enter critical section
	SIMC_Thread_EnterCriticalSection();

	//Loop through entire list of threads to find the matching POSIX thread ID
	for (thread = &SIMC_Thread_Main; thread != NULL; thread = thread->next) {
		if(thread->posixID == posixID)
		{
			ID = thread->id;
			break;
		}
	}

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();
	return ID;
}


SIMC_THREAD_ID SIMC_Thread_GetUniqueID() {
	return (SIMC_THREAD_ID)pthread_self();
}


int SIMC_Thread_WaitFor(SIMC_THREAD_ID ID) {
	pthread_t pthread;
	SIMC_THREAD *thread;

	//Enter critical section
	SIMC_Thread_EnterCriticalSection();

	//Get thread information pointer
	thread = SIMC_Thread_Internal_GetPointer(ID);

	//Is the thread already dead?
	if (thread == NULL) {
		SIMC_Thread_LeaveCriticalSection();
		return 1;
	}

	//If got this far, the thread is alive => polling returns FALSE
	//if(waitmode == GLFW_NOWAIT)
	//{
		//SIMC_Thread_LeaveCriticalSection();
		//return 0;
	//}

	//Get thread handle
	pthread = thread->posixID;

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();

	//Wait for thread to die
	(void)pthread_join(pthread, NULL);

	return 1;
}


void SIMC_Thread_Kill(SIMC_THREAD_ID ID) {
	SIMC_THREAD *thread;

	//Enter critical section
	SIMC_Thread_EnterCriticalSection();

	//Get thread information pointer
	thread = SIMC_Thread_Internal_GetPointer(ID);
	if (thread == NULL) {
		SIMC_Thread_EnterCriticalSection();
		return;
	}

	//Simply murder the process, no mercy!
	pthread_kill(thread->posixID, SIGKILL);

	//Remove thread from thread list
	SIMC_Thread_Internal_Remove(thread);

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();
}
#endif

void SIMC_Thread_Sleep(double time)
{
	struct timeval  currenttime;
	struct timespec wait;
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	long dt_sec, dt_usec;

	if (time < 1e-3) {
		sched_yield();
		return;
	}

	//Set timeout time, relatvie to current time
	gettimeofday( &currenttime, NULL );
	dt_sec  = (long)time;
	dt_usec = (long)((time - (double)dt_sec) * 1000000.0);
	wait.tv_nsec = (currenttime.tv_usec + dt_usec) * 1000L;
	if (wait.tv_nsec > 1000000000L) 	{
		wait.tv_nsec -= 1000000000L;
		dt_sec++;
	}
	wait.tv_sec  = currenttime.tv_sec + dt_sec;

	//Initialize condition and mutex objects
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	//Do a timed wait
	pthread_mutex_lock(&mutex);
	pthread_cond_timedwait(&cond, &mutex, &wait);
	pthread_mutex_unlock(&mutex);

	//Destroy condition and mutex objects
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
}

#ifndef SIMC_SINGLETHREADED
int SIMC_Thread_GetNumProcessors() {
	return (int)sysconf(_SC_NPROCESSORS_ONLN);
}


//#ifndef SIMC_NATIVE_SRW

#define SIMC_SRW_THRESHOLD	0xFFFF
typedef struct SIMC_SRW_LOCK_TAG {
	SIMC_LOCK_ID write_lock;
	long srw_lock;
} SIMC_SRW_LOCK;


SIMC_SRW_ID SIMC_SRW_Create() {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)malloc(sizeof(SIMC_SRW_LOCK));
	lock->write_lock = SIMC_Lock_Create();
	lock->srw_lock = 0;
	return (SIMC_SRW_ID)lock;
}

void SIMC_SRW_Destroy(SIMC_SRW_ID srwID) {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	SIMC_Lock_Destroy(lock->write_lock);
	free(lock);
}

void SIMC_SRW_EnterRead(SIMC_SRW_ID srwID) {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	while (__sync_fetch_and_add(&lock->srw_lock,1) < 0) {
        __sync_fetch_and_add(&lock->srw_lock,-1);
        pthread_yield();
    }
}

void SIMC_SRW_LeaveRead(SIMC_SRW_ID srwID) {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	__sync_fetch_and_add(&lock->srw_lock,-1);
}

void SIMC_SRW_EnterWrite(SIMC_SRW_ID srwID) {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	SIMC_Lock_Enter(lock->write_lock); //Block other threads from writing
	__sync_fetch_and_add(&lock->srw_lock,-SIMC_SRW_THRESHOLD);
	while ((lock->srw_lock) > -SIMC_SRW_THRESHOLD) { //Wait until read threads finish
        pthread_yield();
    }
}

void SIMC_SRW_LeaveWrite(SIMC_SRW_ID srwID) {
	SIMC_SRW_LOCK* lock = (SIMC_SRW_LOCK*)srwID;
	if ((!srwID) || (srwID == SIMC_THREAD_BAD_ID)) return;

	while ((lock->srw_lock) != -SIMC_SRW_THRESHOLD) { //Block execution if not leaving write lock
		pthread_yield();
	}
	__sync_fetch_and_add(&lock->srw_lock,SIMC_SRW_THRESHOLD);
	SIMC_Lock_Leave(lock->write_lock);
}
//#endif


SIMC_LOCK_ID SIMC_Lock_Create() {
	pthread_mutex_t *mutex;

	//Allocate memory for mutex
	mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if(!mutex) return SIMC_THREAD_BAD_ID;

	//Initialise a mutex object
	(void)pthread_mutex_init(mutex, NULL);
	return (SIMC_LOCK_ID)mutex;
}


void SIMC_Lock_Destroy(SIMC_LOCK_ID lockID) {
	//Destroy the mutex object
	pthread_mutex_destroy((pthread_mutex_t*)lockID);
	//Free memory for mutex object
	free((void*)lockID);
}


SIMC_LOCK_ID SIMC_Lock_Enter(SIMC_LOCK_ID lockID) {
	//Wait for mutex to be released
	(void)pthread_mutex_lock((pthread_mutex_t*)lockID);
	return lockID;
}


void SIMC_Lock_Leave(SIMC_LOCK_ID lockID) {
	//Release mutex
	pthread_mutex_unlock((pthread_mutex_t*)lockID);
}


void SIMC_Lock_WaitFor(SIMC_LOCK_ID lockID) {
	SIMC_Lock_Leave(SIMC_Lock_Enter(lockID));
}


void SIMC_Thread_Initialize() {
	if (SIMC_Instance_Counter == 0) {
		// Initialize critical section handle
		(void) pthread_mutex_init(&SIMC_Thread_CriticalSection, NULL );
	
		// The first thread (the main thread) has ID 0
		SIMC_Thread_NextID = 0;
	
		// Fill out information about the main thread (this thread)
		SIMC_Thread_Main.id       = SIMC_Thread_NextID++;
		SIMC_Thread_Main.function = NULL;
		SIMC_Thread_Main.previous = NULL;
		SIMC_Thread_Main.next     = NULL;
		SIMC_Thread_Main.posixID  = pthread_self();
	}
	SIMC_Instance_Counter++;
}


void SIMC_Thread_Deinitialize() {
	SIMC_THREAD *thread, *thread_next;

	//Check instance counter
	SIMC_Instance_Counter--;
	if (SIMC_Instance_Counter > 0) return;

	//Enter critical section
	SIMC_Thread_EnterCriticalSection();

	//Kill all threads
	thread = SIMC_Thread_Main.next;
	while (thread != NULL) {
		//Get pointer to next thread
		thread_next = thread->next;

		//Simply murder the process, no mercy!
		pthread_kill(thread->posixID, SIGKILL);

		//Free memory allocated for this thread
		free((void*)thread);

		//Select next thread in list
		thread = thread_next;
	}

	//Leave critical section
	SIMC_Thread_LeaveCriticalSection();

	//Delete critical section handle
	pthread_mutex_destroy(&SIMC_Thread_CriticalSection);
}
#endif

#endif
