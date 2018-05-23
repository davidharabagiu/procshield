#ifndef _DRIVER_INCLUDE_H_
#define _DRIVER_INCLUDE_H_
//
//  Copyright (C) 2017 BitDefender S.R.L.
//  Author(s)   : David HARABAGIU(dharabagiu@bitdefender.com)
//

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntstrsafe.h>

#define MyDebugPrint(_format, ...) \
    DbgPrintEx(DPFLTR_SYSTEM_ID, DPFLTR_ERROR_LEVEL, _format, __VA_ARGS__)

typedef
NTSTATUS
(NTAPI *PFUNC_ZwQueryInformationProcess) (
    _In_      HANDLE           ProcessHandle,
    _In_      PROCESSINFOCLASS ProcessInformationClass,
    _Out_     PVOID            ProcessInformation,
    _In_      ULONG            ProcessInformationLength,
    _Out_opt_ PULONG           ReturnLength
    );

#endif//_DRIVER_INCLUDE_H_
