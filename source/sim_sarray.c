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

//Elements in a single block
#define ELEMENTS_PER_BLOCK 512


////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////
void SIMC_StorageArray_Create(SIMC_STORAGEARRAY** p_arr, int element_size) {
	SIMC_STORAGEARRAY* arr = (SIMC_STORAGEARRAY*)malloc(sizeof(SIMC_STORAGEARRAY));
	arr->blocks = (void**)malloc(sizeof(void*));
	arr->blocks_count = 0;
	arr->element_count = 0;
	arr->element_size = element_size;
	*p_arr = arr;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////
void SIMC_StorageArray_Destroy(SIMC_STORAGEARRAY* arr) {
	int i;
	for (i = 0; i < arr->blocks_count; i++) free(arr->blocks[i]);
	free(arr->blocks);
	free(arr);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////
void* SIMC_StorageArray_Add(SIMC_STORAGEARRAY* arr) {
	int target_block_index;
	int index_in_block;

	//Add new element
	target_block_index = arr->element_count / ELEMENTS_PER_BLOCK;
	index_in_block = arr->element_count % ELEMENTS_PER_BLOCK;
	arr->element_count++;

	//See if any new blocks must be added
	if (target_block_index >= arr->blocks_count) {
		int index = arr->blocks_count++; //Grow one block
		arr->blocks = (void**)realloc(arr->blocks,sizeof(void**)*arr->blocks_count);
		arr->blocks[index] = (void*)malloc(arr->element_size * ELEMENTS_PER_BLOCK);
	}

	//Add element to the latest block
	return (void*)((char*)arr->blocks[target_block_index] + index_in_block*arr->element_size);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////
void* SIMC_StorageArray_Get(SIMC_STORAGEARRAY* arr, int index) {
	int target_block_index;
	int index_in_block;

	//Add new element
	target_block_index = index / ELEMENTS_PER_BLOCK;
	index_in_block = index % ELEMENTS_PER_BLOCK;

	//Return pointer to element
	return (void*)((char*)arr->blocks[target_block_index] + index_in_block*arr->element_size);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////
void* SIMC_StorageArray_GetAllAndDestroy(SIMC_STORAGEARRAY* arr) {
	int i;
	void* all_data = malloc(arr->element_size * arr->element_count);

	for (i = 0; i < arr->element_count; i++) { //FIXME: copy by blocks
		memcpy((char*)all_data + i*arr->element_size,SIMC_StorageArray_Get(arr,i),arr->element_size);
	}
	SIMC_StorageArray_Destroy(arr);
	return all_data;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////
int SIMC_StorageArray_Count(SIMC_STORAGEARRAY* arr) {
	return arr->element_count;
}