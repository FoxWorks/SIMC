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
#include "sim_core.h"

#ifdef _WIN32
#include <windows.h>
#include <time.h>

double SIMC_Thread_TimeRes = 0.0; //resolution
unsigned __int64 SIMC_Thread_Time64 = 0; //initial time
int SIMC_Thread_TimeInitialized = 0; //initialized
double EVDS_T0_MJD; //MJD date
double EVDS_T0_Time; //MJD time for which date is specified

void SIMC_Thread_Internal_TimeInitialize()
{
	extern double SIMC_Thread_GetTime();
	unsigned __int64 frequency;
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
		SIMC_Thread_TimeRes = 1.0 / (double)frequency;
		SIMC_Thread_TimeInitialized = 1;
		QueryPerformanceCounter((LARGE_INTEGER*)&SIMC_Thread_Time64);
	}
	EVDS_T0_MJD = (time(0) / 86400.0) + 2440587.5 - 2400000.5;;
	EVDS_T0_Time = SIMC_Thread_GetTime();
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Get precise timer value.
///
/// Uses the most precise timer source available on the current system. Maps to
/// QueryPerfomanceCounter() under Windows, and gettimeofday() under Unix-like
/// operating systems.
///
/// @returns Precise timer value in seconds (double-precision)
////////////////////////////////////////////////////////////////////////////////
double SIMC_Thread_GetTime()
{
	unsigned __int64 t_64;
	if (!SIMC_Thread_TimeInitialized) SIMC_Thread_Internal_TimeInitialize();
	if (QueryPerformanceCounter((LARGE_INTEGER*)&t_64)) {
		return (double)(t_64 - SIMC_Thread_Time64)*SIMC_Thread_TimeRes;
	} else {
		return 0.0;
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Returns current time and date, in mean julian date format
///
/// Uses time(0) and SIMC_Thread_GetTime() calls to calculate precise MJD date.
/// Absolute offset between date and real date depends on the computer clock.
///
/// @returns Current date and time in MJD, double precision
////////////////////////////////////////////////////////////////////////////////
double SIMC_Thread_GetMJDTime() {
	return (SIMC_Thread_GetTime() - EVDS_T0_Time)/86400.0 + EVDS_T0_MJD;
}

#else

#include <sys/time.h>
#include <time.h>

double SIMC_Thread_TimeRes = 1.0; //resolution
long long int SIMC_Thread_Time64 = 0; //initial time
int SIMC_Thread_TimeInitialized = 0; //initialized
double EVDS_T0_MJD; //MJD date
double EVDS_T0_Time; //MJD time for which date is specified

void SIMC_Thread_Internal_TimeInitialize()
{
	extern double SIMC_Thread_GetTime();
	struct timeval  tv;

	SIMC_Thread_TimeRes = 1e-6; //1 usec resolution

	gettimeofday(&tv, 0);
	SIMC_Thread_Time64 = (long long int)tv.tv_sec*(long long int)1000000 + (long long int) tv.tv_usec;
	SIMC_Thread_TimeInitialized = 1;

	EVDS_T0_MJD = (time(0) / 86400.0) + 2440587.5 - 2400000.5;;
	EVDS_T0_Time = SIMC_Thread_GetTime();
}

double SIMC_Thread_GetTime()
{
	long long int t0;
	struct timeval  tv;
	if (!SIMC_Thread_TimeInitialized) SIMC_Thread_Internal_TimeInitialize();

	gettimeofday(&tv, 0);
	t0 = (long long int)tv.tv_sec*(long long int)1000000 + (long long int) tv.tv_usec;

	return (double)(t0 - SIMC_Thread_Time64)*SIMC_Thread_TimeRes;
}

double SIMC_Thread_GetMJDTime() {
	return (SIMC_Thread_GetTime() - EVDS_T0_Time)/86400.0 + EVDS_T0_MJD;
}

#endif
