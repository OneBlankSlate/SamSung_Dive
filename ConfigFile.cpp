#include "pch.h"
#include "ConfigFile.h"

CConfigFile::CConfigFile()
{
	InitConfigFile();
}

CConfigFile::~CConfigFile()
{

}
//��ʼ�������ļ�
BOOL CConfigFile::InitConfigFile()
{
	TCHAR  FileFullPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, FileFullPath, MAX_PATH);   //��õ�ǰ��ִ���ļ��ľ���·��

	TCHAR* v1 = NULL;


	//Z:\_CConfigFile\Debug\_CConfigFile.exe
	//Z:\_CConfigFile\Debug\_CConfigFile.ini
	v1 = _tcsstr(FileFullPath, _T("."));   //����   strstr
	if (v1 != NULL)
	{
		*v1 = _T('\0');
		_tcscat_s(FileFullPath, _T(".ini"));
	}
	//����·�������ļ�����
	//����һ��ini�ļ�
	HANDLE FileHandle = CreateFile(FileFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,   //����� ����д
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, NULL);   //ͬ��  �첽   

	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	//����ini�ļ�·�������Ա������
	m_FileFullPath = FileFullPath;

	//�����ļ���С
	ULONG HighLength = 0;
	ULONG LowLength = GetFileSize(FileHandle, &HighLength);  //����ļ��Ĵ�С
	if (LowLength > 0 || HighLength > 0)   //���ǿ��ļ�  
	{

		//���ǵ�һ�δ򿪸��ļ�
		CloseHandle(FileHandle);
		return FALSE;
	}
	//�ļ������������˵�һ��

	//[Key]
	//SubKey = KeyValue
	BOOL LastError = WritePrivateProfileString(_T("Settings"), _T("ListenPort"), _T("2356"), FileFullPath);

	if (LastError == FALSE)
	{
		//��ȡд����ʧ��ԭ��
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
//ʹ��C++�쳣���ƶ�ini��д���ʲ���
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

	//Ini�ļ��е��������ı�����
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


