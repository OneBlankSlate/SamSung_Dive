#pragma once
#include <Winbase.h>
#include <Windows.h>
#include <iostream>
#include <tchar.h>
using namespace std;
#include <Vfw.h>
#pragma comment(lib,"Vfw32.lib")
#pragma pack(1)   //ע��ṹ�������ȶ���
typedef struct  _LOGIN_INFORMAITON_
{
	BYTE			IsToken;		    //��Ϣͷ��
	OSVERSIONINFOEX	OsVersionInfoEx;	//�汾��Ϣ
	char ProcessorName[MAX_PATH];	    //CPU��Ƶ
	IN_ADDR			ClientAddress;		//�洢32λ��IPv4�ĵ�ַ���ݽṹ
	char			HostName[MAX_PATH];	//������
	BOOL			IsWebCameraExist;   //�Ƿ�������ͷ
	DWORD			WebSpeed;		    //����
}LOGIN_INFORMAITON, * PLOGIN_INFORMAITON;
