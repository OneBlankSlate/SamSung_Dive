#include"pch.h"
#include "_VMMap.h"


_CVMMap::_CVMMap()
{

}


_CVMMap::~_CVMMap()
{
	if (m_ProcessHandle != INVALID_HANDLE_VALUE)
	{

		SeCloseHandle(m_ProcessHandle);
		m_ProcessHandle = INVALID_HANDLE_VALUE;
	}
}

void _CVMMap::GetSystemInfo()
{
	::GetSystemInfo(&SystemInfo);
}
VOID _CVMMap::GetMemoryStatus()
{
	GlobalMemoryStatus(&this->MemoryStatus);
}

BOOL _CVMMap::SeEnableSeDebugPrivilege(HANDLE ProcessHandle, BOOL IsEnable)
{
	DWORD  LastError;
	HANDLE TokenHandle = 0;

	if (!OpenProcessToken(ProcessHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))
	{
		LastError = GetLastError();
		if (TokenHandle)
			CloseHandle(TokenHandle);
		return LastError;
	}
	TOKEN_PRIVILEGES TokenPrivileges;
	memset(&TokenPrivileges, 0, sizeof(TOKEN_PRIVILEGES));
	LUID v1;
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &v1))
	{
		LastError = GetLastError();
		CloseHandle(TokenHandle);
		return LastError;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Luid = v1;
	if (IsEnable)
		TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		TokenPrivileges.Privileges[0].Attributes = 0;
	AdjustTokenPrivileges(TokenHandle, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	LastError = GetLastError();
	CloseHandle(TokenHandle);
	return LastError;
}
HANDLE _CVMMap::SeOpenProcess(DWORD DesiredAccess, BOOL IsInheritHandle, HANDLE ProcessID)
{
	if (m_EnableDebugPrivilege)
	{
		SeEnableSeDebugPrivilege(GetCurrentProcess(), TRUE);
	}
	HANDLE ProcessHandle = OpenProcess(DesiredAccess, IsInheritHandle, (DWORD)ProcessID);

	DWORD LastError = GetLastError();
	if (m_EnableDebugPrivilege)
	{
		SeEnableSeDebugPrivilege(GetCurrentProcess(), FALSE);
	}
	SetLastError(LastError);
	return ProcessHandle;
}
BOOL _CVMMap::SeCloseHandle(HANDLE HandleValue)
{
	//
	DWORD HandleFlags;
	if (GetHandleInformation(HandleValue, &HandleFlags)
		&& (HandleFlags & HANDLE_FLAG_PROTECT_FROM_CLOSE) != HANDLE_FLAG_PROTECT_FROM_CLOSE)
		return !!CloseHandle(HandleValue);
	return FALSE;
}
BOOL _CVMMap::GetMemoryBasicInfo(HANDLE ProcessIdentity)
{
	// ��������ڴ���Ϣ����
	this->MemoryBasicInfoList.clear();

	//����Ľ���Identity�����Լ�
	if ((DWORD)ProcessIdentity != GetCurrentProcessId())
	{
		// ��Ŀ�����
		m_ProcessHandle = SeOpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, ProcessIdentity);
		if (m_ProcessHandle == NULL)
		{
			return FALSE;
		}
	}

	MEMORY_BASIC_INFORMATION	MemoryBasicInfo = {0};
	DWORD MemoryBasicInfoLength = sizeof(MEMORY_BASIC_INFORMATION);


	ULONG_PTR StartAddress = (ULONG_PTR)this->SystemInfo.lpMinimumApplicationAddress;
	while (StartAddress < (ULONG_PTR)this->SystemInfo.lpMaximumApplicationAddress)
	{
		// ��ѯָ�����̵�ָ����ַ��״̬��Ϣ
		VirtualQueryEx(m_ProcessHandle, (LPVOID)StartAddress, &MemoryBasicInfo, MemoryBasicInfoLength);
		// ��״̬��Ϣ��ӵ�����
		this->MemoryBasicInfoList.push_back(MemoryBasicInfo);
		// ��λ����һ������
		StartAddress = (ULONG_PTR)MemoryBasicInfo.BaseAddress + MemoryBasicInfo.RegionSize;
	}

	if (m_ProcessHandle != INVALID_HANDLE_VALUE)
	{

		SeCloseHandle(m_ProcessHandle);
		m_ProcessHandle = INVALID_HANDLE_VALUE;
	}
	return (this->MemoryBasicInfoList.size()>0);
}
