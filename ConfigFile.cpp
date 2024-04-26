#include "pch.h"
#include "ConfigFile.h"

CConfigFile::CConfigFile()
{
	InitConfigFile();
}

CConfigFile::~CConfigFile()
{

}
//初始化配置文件
BOOL CConfigFile::InitConfigFile()
{
	TCHAR  FileFullPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, FileFullPath, MAX_PATH);   //获得当前可执行文件的绝对路径

	TCHAR* v1 = NULL;


	//Z:\_CConfigFile\Debug\_CConfigFile.exe
	//Z:\_CConfigFile\Debug\_CConfigFile.ini
	v1 = _tcsstr(FileFullPath, _T("."));   //查找   strstr
	if (v1 != NULL)
	{
		*v1 = _T('\0');
		_tcscat_s(FileFullPath, _T(".ini"));
	}
	//根据路径创建文件对象
	//创建一个ini文件
	HANDLE FileHandle = CreateFile(FileFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,   //共享读 共享写
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, NULL);   //同步  异步   

	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	//保存ini文件路径到类成员变量中
	m_FileFullPath = FileFullPath;

	//计算文件大小
	ULONG HighLength = 0;
	ULONG LowLength = GetFileSize(FileHandle, &HighLength);  //获得文件的大小
	if (LowLength > 0 || HighLength > 0)   //不是空文件  
	{

		//不是第一次打开该文件
		CloseHandle(FileHandle);
		return FALSE;
	}
	//文件被创建出来了第一次

	//[Key]
	//SubKey = KeyValue
	BOOL LastError = WritePrivateProfileString(_T("Settings"), _T("ListenPort"), _T("2356"), FileFullPath);

	if (LastError == FALSE)
	{
		//获取写操作失败原因
		LastError = GetLastError();
	}
	LastError = WritePrivateProfileString(_T("Settings"), _T("MaxConnections"), _T("10"), FileFullPath);
	if (LastError == FALSE)
	{
		LastError = GetLastError();
	}
	CloseHandle(FileHandle);
	return TRUE;
}
//使用C++异常机制对ini读写访问操作
BOOL CConfigFile::GetIntFromConfigFile(const _tstring& Key,const  _tstring& SubKey, int* BufferData)
{
	BOOL IsOk = TRUE;
	try
	{
		*BufferData = GetPrivateProfileInt(Key.c_str(), SubKey.c_str(), 0, m_FileFullPath.c_str());
	}
	catch (...)
	{
		IsOk = FALSE;
	}
	return IsOk;
}
BOOL CConfigFile::SetIntToConfigFile(const _tstring& Key,const _tstring& SubKey, int BufferData)
{

	//Ini文件中的数据是文本数据
	BOOL IsOk = TRUE;
	try
	{
		TCHAR v1[MAX_PATH];
		_stprintf_s(v1, 260, _T("%d"), BufferData);  
		WritePrivateProfileString(Key.c_str(), SubKey.c_str(), v1, m_FileFullPath.c_str());
	}
	catch (...)
	{
		IsOk = FALSE;
	}
	return IsOk;
}


