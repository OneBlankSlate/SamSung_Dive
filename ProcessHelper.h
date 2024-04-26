#pragma once
#include<Windows.h>
#include<tchar.h>
#include<iostream>
#include <TlHelp32.h>
using namespace std;
#define PAGE_SIZE 0x1000

#define PAGE_READ_FLAGS \
    (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)
#define PAGE_WRITE_FLAGS \
    (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
BOOL SeGetProcessIDByProcessImageName(TCHAR* ProcessImageName, HANDLE* ProcessIdentity);
BOOL SeOpenProcessByProcessID(HANDLE ProcessIdentity, HANDLE* ProcessHandle);
BOOL SeIsValidReadPoint(LPVOID VirtualAddress);//0x1000
BOOL SeIsValidWritePoint(LPVOID VirtualAddress);
BOOL SeEnableSeDebugPrivilege(IN const TCHAR* PriviledgeName, BOOL IsEnable);
BOOL SeRemoteMemoryFindFirst(HANDLE ProcessHandle, int DataValue);
BOOL SeRemoteMemoryFindNext(HANDLE ProcessHandle, int DataValue);
BOOL SeRemoteMemoryFix(HANDLE ProcessHandle);

extern size_t __Array[1024];
extern size_t __ArrayCount ;
