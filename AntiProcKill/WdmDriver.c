//
//  Copyright (C) 2017 BitDefender S.R.L.
//  Author(s)   : David HARABAGIU(dharabagiu@bitdefender.com)
//

#include "DriverInclude.h"
#include "Callbacks.h"

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

NTSTATUS
CbFilterUnload(
    FLT_FILTER_UNLOAD_FLAGS Flags
);

PFLT_FILTER gFilter = NULL;
PVOID gObRegistrationHandle = NULL;

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    //DbgBreakPoint();

    NTSTATUS status = STATUS_DEVICE_NOT_READY;
    OB_CALLBACK_REGISTRATION obCbRegistration = { 0 };
    OB_OPERATION_REGISTRATION obOpRegistration = { 0 };
    UNICODE_STRING cbAltitude = { 0 };
    UNICODE_STRING functionName = { 0 };
    FLT_REGISTRATION registration = { 0 };
    FLT_CONTEXT_REGISTRATION contextRegistration[] = { { FLT_CONTEXT_END } };
    FLT_OPERATION_REGISTRATION operationRegistration[] = {
        {
            IRP_MJ_CREATE,
            0,
            CbPreCreate,
            CbPostCreate,
            NULL
        },
        {
            IRP_MJ_SET_INFORMATION,
            0,
            CbPreSetInfo,
            CbPostSetInfo,
            NULL
        },
        { IRP_MJ_OPERATION_END }
    };
    UNREFERENCED_PARAMETER(RegistryPath);
    
    registration.Size = sizeof(FLT_REGISTRATION);
    registration.Version = FLT_REGISTRATION_VERSION;
    registration.Flags = 0;
    registration.ContextRegistration = contextRegistration;
    registration.OperationRegistration = operationRegistration;
    registration.FilterUnloadCallback = CbFilterUnload;
    registration.InstanceSetupCallback = NULL;
    registration.InstanceQueryTeardownCallback = NULL;
    registration.InstanceTeardownStartCallback = NULL;
    registration.InstanceTeardownCompleteCallback = NULL;
    registration.GenerateFileNameCallback = NULL;
    registration.NormalizeNameComponentCallback = NULL;
    registration.NormalizeContextCleanupCallback = NULL;
    registration.TransactionNotificationCallback = NULL;
    registration.NormalizeNameComponentExCallback = NULL;

    RtlInitUnicodeString(&cbAltitude, L"1000");
    RtlInitUnicodeString(&functionName, L"ZwQueryInformationProcess");

    obOpRegistration.ObjectType = PsProcessType;
    obOpRegistration.Operations = OB_OPERATION_HANDLE_CREATE;
    obOpRegistration.PreOperation = CbPreProcessCreate;
    obOpRegistration.PostOperation = CbPostProcessCreate;
    
    obCbRegistration.Version = OB_FLT_REGISTRATION_VERSION;
    obCbRegistration.OperationRegistrationCount = 1;
    obCbRegistration.Altitude = cbAltitude;
    obCbRegistration.RegistrationContext = NULL;
    obCbRegistration.OperationRegistration = &obOpRegistration;

    __try
    {
        status = FltRegisterFilter(DriverObject, &registration, &gFilter);
        if (!NT_SUCCESS(status))
        {
            MyDebugPrint("FltRegisterFilter failed with status %l\n", status);
            __leave;
        }

        status = FltStartFiltering(gFilter);
        if (!NT_SUCCESS(status))
        {
            MyDebugPrint("FltStartFiltering failed with status %l\n", status);
            __leave;
        }

        #pragma warning (suppress:4055)
        ZwQueryInformationProcess = (PFUNC_ZwQueryInformationProcess)MmGetSystemRoutineAddress(&functionName);
        if (!NT_SUCCESS(status))
        {
            MyDebugPrint("MmGetSystemRoutineAddress failed with status %l\n", status);
            __leave;
        }

        status = ObRegisterCallbacks(&obCbRegistration, &gObRegistrationHandle);
        if (!NT_SUCCESS(status))
        {
            MyDebugPrint("ObRegisterCallbacks failed with status %l\n", status);
            __leave;
        }

        status = STATUS_SUCCESS;
    }
    __finally
    {
        
    }

    return status;
}

NTSTATUS
CbFilterUnload(
    FLT_FILTER_UNLOAD_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(Flags);
    ObUnRegisterCallbacks(gObRegistrationHandle);
    FltUnregisterFilter(gFilter);
    return STATUS_SUCCESS;
}
