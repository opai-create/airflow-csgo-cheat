#pragma once
#include <iostream>
#include <Windows.h>
#include <winnt.h>

#define db( _VALUE_ ) __asm __emit ( 0x##_VALUE_ )

#define ENTER_X64_MODE( )                    \
db( 6A ) db( 33 )           \
db( E8 ) db( 00 ) db( 00 ) db( 00 ) db( 00 ) \
db( 83 ) db( 04 ) db( 24 ) db( 05 )       \
db( CB )                      

#define EXIT_X64_MODE( )                                                \
db( E8 ) db( 00 ) db( 00 ) db( 00 ) db( 00 )                            \
db( C7 ) db( 44 ) db( 24 ) db( 04 ) db( 23 ) db( 00 ) db( 00 ) db( 00 ) \
db( 83 ) db( 04 ) db( 24 ) db( 0D )                                     \
db( CB )

typedef VOID(WINAPI* PPS_POST_PROCESS_INIT_ROUTINE)(VOID);

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef struct _PEB_LDR_DATA {
    BYTE       Reserved1[8];
    PVOID      Reserved2[3];
    LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    BYTE           Reserved1[16];
    PVOID          Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB {
    BYTE                          Reserved1[2];
    BYTE                          BeingDebugged;
    BYTE                          Reserved2[1];
    PVOID                         Reserved3[2];
    PPEB_LDR_DATA                 Ldr;
    PRTL_USER_PROCESS_PARAMETERS  ProcessParameters;
    PVOID                         Reserved4[3];
    PVOID                         AtlThunkSListPtr;
    PVOID                         Reserved5;
    ULONG                         Reserved6;
    PVOID                         Reserved7;
    ULONG                         Reserved8;
    ULONG                         AtlThunkSListPtr32;
    PVOID                         Reserved9[45];
    BYTE                          Reserved10[96];
    PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    BYTE                          Reserved11[128];
    PVOID                         Reserved12[1];
    ULONG                         SessionId;
} PEB, * PPEB;

typedef struct _PEB* PPEB;
struct PROCESS_BASIC_INFORMATION {
    long  ExitStatus;
    PPEB  PebBaseAddress;
    unsigned long  AffinityMask;
    long           BasePriority;
    unsigned long  UniqueProcessId;
    unsigned long  InheritedFromUniqueProcessId;
};

using NTSTATUS = LONG;

template< typename... _VA_ARGS_ >
NTSTATUS __cdecl syscall(DWORD index, _VA_ARGS_... args)
{
    NTSTATUS status = NULL;
    DWORD back_esp = NULL;

    const SIZE_T  num_args = sizeof...(args) < 4 ? (4) : sizeof...(args);
    const DWORD64  args_arr[num_args]{ (DWORD64)args... };

    DWORD64 _rcx = args_arr[00];
    DWORD64 _rdx = args_arr[01];
    DWORD64 _r8 = args_arr[02];
    DWORD64 _r9 = args_arr[03];

    DWORD64 stack_arg_num = (num_args > 4) ? (num_args - 4) : (NULL);
    DWORD64 stack_args = (DWORD64)(&args_arr[3]);
    __asm
    {

        push ebx
        push edi
        push esi

        mov back_esp, esp
        and esp, 0x0FFFFFFF0
        ENTER_X64_MODE();
        {
            push _rcx
            pop ecx

            push _rdx
            pop edx

            push _r8
            db(41) db(58)

            push _r9
            db(41) db(59)

            push stack_arg_num
            pop esi

            push stack_args
            pop edi

            test esi, esi
            jz SYSTEM_CALL

            STACK_ARG_PUSH :
        push[edi + esi * 8]
            sub esi, 1
            jnz STACK_ARG_PUSH

            SYSTEM_CALL :

        sub esp, 0x28

            mov eax, index
            db(49) db(89) db(CA) //mov r10, rcx
            db(0F) db(05)       //syscall

            mov status, eax

            add esp, 0x28

            push stack_arg_num
            pop esi
            imul esi, 8
            add esp, esi
        }
        EXIT_X64_MODE();

        mov esp, back_esp

        pop esi
        pop edi
        pop ebx
    }

    return status;
}

