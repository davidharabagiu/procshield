#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_
//
//  Author(s)   : David HARABAGIU(dharabagiu@bitdefender.com)
//

#include "DriverInclude.h"

extern PFUNC_ZwQueryInformationProcess ZwQueryInformationProcess;

OB_PREOP_CALLBACK_STATUS
CbPreProcessCreate(
    _In_ PVOID RegistrationContext,
    _In_ POB_PRE_OPERATION_INFORMATION OperationInformation
);

VOID
CbPostProcessCreate(
    _In_ PVOID RegistrationContext,
    _In_ POB_POST_OPERATION_INFORMATION OperationInformation
);

FLT_PREOP_CALLBACK_STATUS
CbPreCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Out_ PVOID *CompletionContext
);

FLT_POSTOP_CALLBACK_STATUS
CbPostCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);

FLT_PREOP_CALLBACK_STATUS
CbPreSetInfo(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);

FLT_POSTOP_CALLBACK_STATUS
CbPostSetInfo(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);

#endif//_CALLBACKS_H_
