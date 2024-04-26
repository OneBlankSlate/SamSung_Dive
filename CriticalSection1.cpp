#include "pch.h"
#include "CriticalSection1.h"
CCriticalSection1::CCriticalSection1(CRITICAL_SECTION* CriticalSection)
{
	m_CriticalSection = CriticalSection;   //÷∏’Î
	Lock();
}
CCriticalSection1::~CCriticalSection1()
{
	Unlock();
}
void CCriticalSection1::Unlock()
{
	LeaveCriticalSection(m_CriticalSection);
}
void CCriticalSection1::Lock()
{
	EnterCriticalSection(m_CriticalSection);
}
