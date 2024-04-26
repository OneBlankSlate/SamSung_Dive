#pragma once
#include<Windows.h>
#include<iostream>
#include<tchar.h>
using namespace std;
#ifdef UNICODE
#define _tcout wcout
#define _tstring wstring
#define _tcin wcin
#else
#define _tcout cout
#define _tstring string
#define _tcin cin
#endif

class CConfigFile
{
public:
	CConfigFile();
	~CConfigFile();
	BOOL InitConfigFile();
	BOOL GetIntFromConfigFile(const _tstring& Key,const _tstring& SubKey, int* BufferData);
	BOOL SetIntToConfigFile(const _tstring& Key,const _tstring& SubKey, int BufferData);
private:
	_tstring m_FileFullPath;


};

