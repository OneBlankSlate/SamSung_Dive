#include"pch.h"
#include"ProcessHelper.h"

#ifdef _WIN64
#define  RING3_LIMITED 0x00007FFFFFFEFFFF   
#else
#define  RING3_LIMITED 0x7FFEFFFF
#endif // _WIN64

size_t __Array[1024];
size_t __ArrayCount = 0;
BOOL SeGetProcessIDByProcessImageName(TCHAR* ProcessImageName, HANDLE* ProcessIdentity)
{
	//进程快照  
	BOOL IsOk = FALSE;
	HANDLE ToolHelp32Handle = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 ProcessEntry32 = { 0 };
	ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);

	if (SeIsValidReadPoint(ProcessImageName) == FALSE || SeIsValidWritePoint(ProcessIdentity) == FALSE)
	{
		return FALSE;
	}
	ToolHelp32Handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (ToolHelp32Handle == INVALID_HANDLE_VALUE)
	{

		return FALSE;
	}
	if (Process32First(ToolHelp32Handle, &ProcessEntry32) == FALSE)
	{

		CloseHandle(ToolHelp32Handle);
		return FALSE;
	}
	do
	{
		if (_memicmp(ProcessEntry32.szExeFile, ProcessImageName, _tcslen(ProcessImageName) * sizeof(TCHAR)) == 0)
		{

			*ProcessIdentity = (HANDLE)ProcessEntry32.th32ProcessID;
			IsOk = TRUE;
			break;
		}

	} while (Process32Next(ToolHelp32Handle, &ProcessEntry32));
	if (ToolHelp32Handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(ToolHelp32Handle);
	}
	return IsOk;
}
BOOL SeOpenProcessByProcessID(HANDLE ProcessIdentity, HANDLE* ProcessHandle)
{
	if (SeIsValidWritePoint(ProcessHandle) == FALSE)
	{
		return FALSE;
	}
	//提权
	if (SeEnableSeDebugPrivilege(_T("SeDebugPrivilege"), TRUE) == FALSE)
	{
		return FALSE;
	}

	//打开目标进程获得目标进程句柄
	*ProcessHandle = OpenProcess(GENERIC_ALL, FALSE, (DWORD)ProcessIdentity);

	if (*ProcessHandle != INVALID_HANDLE_VALUE)
	{
		SeEnableSeDebugPrivilege(_T("SeDebugPrivilege"), FALSE);
		return TRUE;
	}
	SeEnableSeDebugPrivilege(_T("SeDebugPrivilege"), FALSE);
	return FALSE;
}
BOOL SeIsValidReadPoint(LPVOID VirtualAddress)//0x1000
{
	//try
	//query
	BOOL IsOk = FALSE;
	MEMORY_BASIC_INFORMATION MemoryBasicInfo = { 0 };
	VirtualQuery(VirtualAddress, &MemoryBasicInfo, sizeof(MEMORY_BASIC_INFORMATION));
	if ((MemoryBasicInfo.State == MEM_COMMIT && (MemoryBasicInfo.Protect & PAGE_READ_FLAGS)))
	{
		IsOk = TRUE;
	}
	return IsOk;
}
BOOL SeIsValidWritePoint(LPVOID VirtualAddress)
{
	BOOL IsOk = FALSE;
	MEMORY_BASIC_INFORMATION MemoryBasicInfo = { 0 };
	VirtualQuery(VirtualAddress, &MemoryBasicInfo, sizeof(MEMORY_BASIC_INFORMATION));
	if ((MemoryBasicInfo.State == MEM_COMMIT && (MemoryBasicInfo.Protect & PAGE_WRITE_FLAGS)))
	{
		IsOk = TRUE;
	}
	return IsOk;
}
BOOL SeEnableSeDebugPrivilege(IN const TCHAR* PriviledgeName, BOOL IsEnable)
{
	// 打开权限令牌

	HANDLE  ProcessHandle = GetCurrentProcess();   //获得当前自己的进程句柄
	HANDLE  TokenHandle = NULL;

	//通过进程句柄获得进程令牌句柄
	if (!OpenProcessToken(ProcessHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))  //
	{
		return FALSE;
	}
	LUID			 v1;
	if (!LookupPrivilegeValue(NULL, PriviledgeName, &v1))		// 通过权限名称查找uID
	{
		CloseHandle(TokenHandle);
		TokenHandle = NULL;
		return FALSE;
	}
	TOKEN_PRIVILEGES TokenPrivileges = { 0 };
	TokenPrivileges.PrivilegeCount = 1;		// 要提升的权限个数
	TokenPrivileges.Privileges[0].Attributes = IsEnable == TRUE ? SE_PRIVILEGE_ENABLED : 0;    // 动态数组，数组大小根据Count的数目
	TokenPrivileges.Privileges[0].Luid = v1;
	if (!AdjustTokenPrivileges(TokenHandle, FALSE, &TokenPrivileges,
		sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{

		CloseHandle(TokenHandle);
		TokenHandle = NULL;
		return FALSE;
	}
	CloseHandle(TokenHandle);
	TokenHandle = NULL;
	return TRUE;
}
BOOL SeRemoteMemoryFindFirst(HANDLE ProcessHandle, int DataValue)
{
	//获得系统版本
	__int64 VirtualAddress = 0;
	OSVERSIONINFO VersionInfo = { 0 };
	GetVersionEx(&VersionInfo);   //获得系统版本
	if (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		VirtualAddress = 4 * 1024 * 1024;		     // Windows 98系列，4MB	
	else
		VirtualAddress = 64 * 1024;

	// Windows NT系列，64KB
	_MEMORY_BASIC_INFORMATION64 MemoryBasicInfo = { 0 };
	BYTE BufferData[PAGE_SIZE] = { 0 };
	while (VirtualAddress < RING3_LIMITED)
	{
		__int64 BlockSize = 0;
		__int64 NewAddress = -1;

		BlockSize = VirtualQueryEx(ProcessHandle, (LPCVOID)VirtualAddress,
			(PMEMORY_BASIC_INFORMATION)&MemoryBasicInfo, sizeof(_MEMORY_BASIC_INFORMATION64));
		__int64 StartAddress = 0;
		__int64 EndAddress = 0;
		if (BlockSize == sizeof(_MEMORY_BASIC_INFORMATION64))   //目标进程是64位
		{
			NewAddress = (__int64)MemoryBasicInfo.BaseAddress + (__int64)MemoryBasicInfo.RegionSize + 1;   //定位到下一个区域

			if (!(MemoryBasicInfo.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
			{
				StartAddress = MemoryBasicInfo.BaseAddress;                                     //当前内存页可以写
				EndAddress = MemoryBasicInfo.BaseAddress + MemoryBasicInfo.RegionSize;
			}
		}
		else if (BlockSize == sizeof(_MEMORY_BASIC_INFORMATION32))  //目标进程是32位
		{
			_MEMORY_BASIC_INFORMATION32* MemoryBasicInfo32 = (_MEMORY_BASIC_INFORMATION32*)&MemoryBasicInfo;
			NewAddress = (__int64)MemoryBasicInfo32->BaseAddress + (__int64)MemoryBasicInfo32->RegionSize + 1;

			if (!(MemoryBasicInfo32->Protect & (PAGE_NOACCESS | PAGE_GUARD)))
			{
				StartAddress = MemoryBasicInfo32->BaseAddress;
				EndAddress = MemoryBasicInfo32->BaseAddress + MemoryBasicInfo32->RegionSize;
			}
		}
		if (StartAddress > 0 && EndAddress > 0)
		{
			StartAddress = StartAddress - (StartAddress % PAGE_SIZE);
			SIZE_T ReturnLength = 0;
			while (StartAddress < EndAddress)
			{
				if (ReadProcessMemory(ProcessHandle, (LPCVOID)((unsigned char*)StartAddress), BufferData, 0x1000, &ReturnLength)
					&& ReturnLength == PAGE_SIZE)
				{
					__int64* v1;
					for (int i = 0; i < (int)4 * 1024 - 3; i++)
					{
						v1 = (__int64*)&BufferData[i];
						if (v1[0] == DataValue)	// 等于要查找的值？
						{
							if (__ArrayCount >= 1024)
								return FALSE;   //如果有需求使用动态
							// 添加到全局变量中
							__Array[__ArrayCount++] = StartAddress + i;
						}
					}
				}

				StartAddress += PAGE_SIZE;
			}
		}
		if (NewAddress <= VirtualAddress)
			break;
		VirtualAddress = NewAddress;
	}

	if (__ArrayCount == 0)
	{
		return FALSE;
	}

	return TRUE;

}
BOOL SeRemoteMemoryFindNext(HANDLE ProcessHandle, int DataValue)
{
	// 保存m_arList数组中有效地址的个数，初始化新的m_nListCnt值
	int v1 = __ArrayCount;   //[][]
	__ArrayCount = 0;

	// 在m_arList数组记录的地址处查找
	BOOL IsOk = FALSE;	// 假设失败	
	DWORD v3 = 0;
	for (int i = 0; i < v1; i++)
	{
		if (ReadProcessMemory(ProcessHandle, (LPVOID)__Array[i], &v3, sizeof(DWORD), NULL))
		{
			if (v3 == DataValue)
			{
				__Array[__ArrayCount++] = __Array[i];
				IsOk = TRUE;
			}
		}
	}

	return IsOk;
}
BOOL SeRemoteMemoryFix(HANDLE ProcessHandle)
{
	printf("Input New Value To Fix\r\n");
	int DataValue = 0;
	scanf("%d", &DataValue);

	// 写入新值
	if (WriteProcessMemory(ProcessHandle, (LPVOID)__Array[0], &DataValue, sizeof(DWORD), NULL) == TRUE)
	{
		return TRUE;
	}
	return FALSE;
}

