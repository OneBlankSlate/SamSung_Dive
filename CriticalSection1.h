#pragma once
class CCriticalSection1   //辅助临界区使用
{
public:
	CCriticalSection1(CRITICAL_SECTION* CriticalSection);
	~CCriticalSection1();
	void Lock();			 //进入临界区
	void Unlock();			 //离开临界区
private:
	CRITICAL_SECTION* m_CriticalSection;        //指针，指向真正的临界区
};

