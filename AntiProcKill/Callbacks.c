//
//  Author(s)   : David HARABAGIU(dharabagiu@bitdefender.com)
//

#include "Callbacks.h"
#include "Misc.h"
#include "DriverInclude.h"

#define MAX_IMAGE_NAME 1024
#define PROCESS_TERMINATE 1

PFUNC_ZwQueryInformationProcess ZwQueryInformationProcess;

OB_PREOP_CALLBACK_STATUS
CbPreProcessCreate(
    _In_ PVOID RegistrationContext,
    _In_ POB_PRE_OPERATION_INFORMATION OperationInformation
)
{
    static UNICODE_STRING prcNameExplorer = RTL_CONSTANT_STRING(L"explorer.exe");
    static UNICODE_STRING prcNameTaskmgr = RTL_CONSTANT_STRING(L"taskmgr.exe");
    static UNICODE_STRING prcNameProcmon = RTL_CONSTANT_STRING(L"procmon.exe");
    static UNICODE_STRING prcNameProcmon64 = RTL_CONSTANT_STRING(L"procmon64.exe");
    static UNICODE_STRING prcNameProcexp = RTL_CONSTANT_STRING(L"procexp.exe");
    static UNICODE_STRING prcNameProcexp64 = RTL_CONSTANT_STRING(L"procexp64.exe");

    if (OperationInformation->KernelHandle == TRUE)
    {
        return OB_PREOP_SUCCESS;
    }

    NTSTATUS status = STATUS_SUCCESS;
    HANDLE hProcess;
    WCHAR imageNameBuffer[sizeof(UNICODE_STRING) / sizeof(WCHAR) + MAX_IMAGE_NAME];
    PUNICODE_STRING imageName = (PUNICODE_STRING)&imageNameBuffer;
    UNREFERENCED_PARAMETER(RegistrationContext);

    imageName->Buffer = &imageNameBuffer[sizeof(UNICODE_STRING) / sizeof(WCHAR)];
    imageName->Length = 0;
    imageName->MaximumLength = MAX_IMAGE_NAME * sizeof(WCHAR);

    status = ObOpenObjectByPointer(
        OperationInformation->Object,
        OBJ_KERNEL_HANDLE,
        NULL,
        0,
        NULL,
        KernelMode,
        &hProcess
    );
    if (!NT_SUCCESS(status))
    {
        MyDebugPrint("ObOpenObjectByPointer failed with status %l\n", status);
        return OB_PREOP_SUCCESS;
    }

    status = ZwQueryInformationProcess(
        hProcess,
        ProcessImageFileName,
        imageName,
        sizeof(imageNameBuffer),
        NULL
    );
    if (!NT_SUCCESS(status))
    {
        MyDebugPrint("ZwQueryInformationProcess failed with status %l\n", status);
        return OB_PREOP_SUCCESS;
    }

    status = ObCloseHandle(hProcess, KernelMode);
    if (!NT_SUCCESS(status))
    {
        MyDebugPrint("ObCloseHandle failed with status %l\n", status);
        return OB_PREOP_SUCCESS;
    }

    if (imageName->Length == 0)
    {
        return OB_PREOP_SUCCESS;
    }

    UNICODE_STRING fileName = GetFileNameFromPath(imageName);

    if (RtlEqualUnicodeString(&fileName, &prcNameExplorer, TRUE) ||
        RtlEqualUnicodeString(&fileName, &prcNameTaskmgr, TRUE) ||
        RtlEqualUnicodeString(&fileName, &prcNameProcmon, TRUE) ||
        RtlEqualUnicodeString(&fileName, &prcNameProcmon64, TRUE) ||
        RtlEqualUnicodeString(&fileName, &prcNameProcexp, TRUE) ||
        RtlEqualUnicodeString(&fileName, &prcNameProcexp64, TRUE))
    {
        OperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;
    }

    return OB_PREOP_SUCCESS;
}

VOID
CbPostProcessCreate(
    _In_ PVOID RegistrationContext,
    _In_ POB_POST_OPERATION_INFORMATION OperationInformation
)
{
    UNREFERENCED_PARAMETER(RegistrationContext);
    UNREFERENCED_PARAMETER(OperationInformation);
}

FLT_PREOP_CALLBACK_STATUS
CbPreCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Out_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    if (FlagOn(Data->Iopb->Parameters.Create.Options, FILE_DELETE_ON_CLOSE))
    {
        BackupFile(Data, FltObjects);
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
CbPostCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS
CbPreSetInfo(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformation)
    {
        if (((PFILE_DISPOSITION_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->DeleteFile == TRUE)
        {
            BackupFile(Data, FltObjects);
        }
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
CbPostSetInfo(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    return FLT_POSTOP_FINISHED_PROCESSING;
}