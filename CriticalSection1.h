#pragma once
class CCriticalSection1   //�����ٽ���ʹ��
{
public:
	CCriticalSection1(CRITICAL_SECTION* CriticalSection);
	~CCriticalSection1();
	void Lock();			 //�����ٽ���
	void Unlock();			 //�뿪�ٽ���
private:
	CRITICAL_SECTION* m_CriticalSection;        //ָ�룬ָ���������ٽ���
};

