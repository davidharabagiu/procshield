//
//  Copyright (C) 2017 BitDefender S.R.L.
//  Author(s)   : David HARABAGIU(dharabagiu@bitdefender.com)
//

#include "Misc.h"
#include "DriverInclude.h"

#define BUFSIZE 1024
#define TAG_APKS 'SKPA'

UNICODE_STRING
GetFileNameFromPath(
    _In_ PCUNICODE_STRING Path
)
{
    UNICODE_STRING fileName;
    fileName.Length = 0;
    fileName.MaximumLength = Path->MaximumLength;
    fileName.Buffer = NULL;

    if (Path->Length == 0)
    {
        return fileName;
    }

    INT offset = Path->Length - sizeof(WCHAR);
    while (Path->Buffer[offset / sizeof(WCHAR)] != L'\\' && offset >= 0)
    {
        offset -= sizeof(WCHAR);
        fileName.Length += sizeof(WCHAR);
    }
    fileName.Buffer = &Path->Buffer[offset / sizeof(WCHAR) + 1];

    return fileName;
}

NTSTATUS
BackupFile(
    _In_ const PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects
)
{
    static UNICODE_STRING backupDirectoryName = RTL_CONSTANT_STRING(L"\\DosDevices\\C:\\AntiProcKill Backup");
    static ULONG seed = 1;

    NTSTATUS status = STATUS_SUCCESS;
    HANDLE hBackupDirectory;
    HANDLE hBackupFile;
    UNICODE_STRING backupFileName;
    OBJECT_ATTRIBUTES attrBackupDirectory;
    OBJECT_ATTRIBUTES attrBackupFile;
    IO_STATUS_BLOCK ioStatusBlock;
    UCHAR buffer[BUFSIZE];
    PFLT_FILE_NAME_INFORMATION fileNameInformation = NULL;

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED, &fileNameInformation);
    if (!NT_SUCCESS(status))
    {
        MyDebugPrint("FltGetFileNameInformation failed with status 0x%X\n", status);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    status = FltParseFileNameInformation(fileNameInformation);
    if (!NT_SUCCESS(status))
    {
        MyDebugPrint("FltParseFileNameInformation failed with status 0x%X\n", status);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    ULONG randomNumber = RtlRandomEx(&seed);

    backupFileName.Buffer = ExAllocatePoolWithTag(
        NonPagedPool,
        backupDirectoryName.Length + fileNameInformation->FinalComponent.Length + 15 * sizeof(WCHAR),
        TAG_APKS
    );
    backupFileName.MaximumLength = backupDirectoryName.Length + fileNameInformation->FinalComponent.Length + 15 * sizeof(WCHAR);

    status = RtlUnicodeStringPrintf(&backupFileName,
        L"%wZ\\%lu_%wZ",
        &backupDirectoryName,
        randomNumber,
        &fileNameInformation->FinalComponent
    );
    if (!NT_SUCCESS(status))
    {
        MyDebugPrint("RtlUnicodeStringPrintf failed with status 0x%X\n", status);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    InitializeObjectAttributes(
        &attrBackupDirectory,
        &backupDirectoryName,
        OBJ_KERNEL_HANDLE,
        NULL,
        NULL
    );

    InitializeObjectAttributes(
        &attrBackupFile,
        &backupFileName,
        OBJ_KERNEL_HANDLE,
        NULL,
        NULL
    );

    status = ZwCreateFile(
        &hBackupDirectory,
        GENERIC_READ | GENERIC_WRITE,
        &attrBackupDirectory,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        0,
        FILE_OPEN_IF,
        FILE_DIRECTORY_FILE,
        NULL,
        0
    );
    if (!NT_SUCCESS(status))
    {
        MyDebugPrint("ZwCreateFile failed with status 0x%X\n", status);
        return status;
    }

    status = ZwClose(hBackupDirectory);
    if (!NT_SUCCESS(status))
    {
        MyDebugPrint("ZwClose failed with status 0x%X\n", status);
        return status;
    }

    status = ZwCreateFile(
        &hBackupFile,
        GENERIC_WRITE,
        &attrBackupFile,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        0,
        FILE_OVERWRITE_IF,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
    );
    if (!NT_SUCCESS(status))
    {
        MyDebugPrint("ZwCreateFile failed with status 0x%X\n", status);
        return status;
    }

    ULONG bytesRead = 0;

    do
    {
        status = FltReadFile(
            FltObjects->Instance,
            FltObjects->FileObject,
            NULL,
            BUFSIZE * sizeof(UCHAR),
            buffer,
            0,
            &bytesRead,
            NULL,
            NULL
        );
        if (!NT_SUCCESS(status))
        {
            MyDebugPrint("FltReadFile failed with status 0x%X\n", status);
            goto cleanup;
        }

        if (bytesRead == 0)
        {
            break;
        }

        ioStatusBlock.Information = 0;
        ioStatusBlock.Pointer = NULL;
        ioStatusBlock.Status = STATUS_SUCCESS;

        status = ZwWriteFile(
            hBackupFile,
            NULL,
            NULL,
            NULL,
            &ioStatusBlock,
            buffer,
            bytesRead,
            NULL,
            NULL
        );
        if (!NT_SUCCESS(status))
        {
            MyDebugPrint("ZwWriteFile failed with status 0x%X\n", status);
            goto cleanup;
        }
    } while (bytesRead != 0);

cleanup:

    status = ZwClose(hBackupFile);
    if (!NT_SUCCESS(status))
    {
        MyDebugPrint("ZwClose failed with status 0x%X\n", status);
        return status;
    }

    ExFreePoolWithTag(backupFileName.Buffer, TAG_APKS);
    FltReleaseFileNameInformation(fileNameInformation);

    return status;
}