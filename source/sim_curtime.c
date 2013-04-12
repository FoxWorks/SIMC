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
