#ifndef __UEFI_BASE_TYPES_H__
#define __UEFI_BASE_TYPES_H__

#include <stdint.h>

typedef uint64_t UINT64;
typedef int64_t INT64;
typedef uint32_t UINT32;
typedef int32_t INT32;
typedef uint16_t UINT16;
typedef uint16_t CHAR16;
typedef int16_t INT16;
typedef uint8_t BOOLEAN;
typedef uint8_t UINT8;
typedef int8_t CHAR8;
typedef int8_t INT8;

typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8 Data4[8];
} GUID;

typedef GUID EFI_GUID;

#endif
