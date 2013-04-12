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
	void** old_read = queue->read_ptr;
	void** old_write = queue->write_ptr;
	int used_slots_;

	if (old_read < old_write) {
		used_slots_ = (int)(old_write-old_read);
	} else {
		used_slots_ = (int)(queue->size-(old_read-old_write));
		if (old_read == old_write) used_slots_ = 0;
	}

	if (free_slots) *free_slots = queue->size - used_slots_;
	if (used_slots) *used_slots = used_slots_;
}
