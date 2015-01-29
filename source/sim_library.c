////////////////////////////////////////////////////////////////////////////////
/// @file
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
#include <stdio.h>
#include "sim_core.h"

#ifdef _WIN32
#include <windows.h>


////////////////////////////////////////////////////////////////////////////////
/// @brief Load a library
////////////////////////////////////////////////////////////////////////////////
SIMC_LIBRARY_ID SIMC_Library_Load(char* library_name) {
	HMODULE handle;
	char full_name[8193];

#ifdef PLATFORM64
	snprintf(full_name, 8192, "%sd", library_name);
#else
	snprintf(full_name, 8192, "%s_32d", library_name);
#endif
	full_name[8192] = 0;

	handle = LoadLibrary(full_name);
	if (handle == NULL) {
#ifdef PLATFORM32
		snprintf(full_name, 8192, "%s_32", library_name);
#endif
		full_name[8192] = 0;
		handle = LoadLibrary(full_name);
	}
	return (SIMC_LIBRARY_ID)handle;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Unload library
////////////////////////////////////////////////////////////////////////////////
void SIMC_Library_Unload(SIMC_LIBRARY_ID library) {
	FreeLibrary(library);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Get pointer to a function
////////////////////////////////////////////////////////////////////////////////
void* SIMC_Library_GetFunction(SIMC_LIBRARY_ID library, char* function_name) {
	return GetProcAddress(library, function_name);
}

#else

// Not supported yet

#endif