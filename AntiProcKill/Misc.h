#ifndef _MISC_H_
#define _MISC_H_
//
//  Copyright (C) 2017 BitDefender S.R.L.
//  Author(s)   : David HARABAGIU(dharabagiu@bitdefender.com)
//

#include "DriverInclude.h"

UNICODE_STRING
GetFileNameFromPath(
    _In_ PCUNICODE_STRING Path
);

NTSTATUS
BackupFile(
    _In_ const PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects
);

#endif//_MISC_H_
