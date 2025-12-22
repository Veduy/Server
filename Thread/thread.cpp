#include <Windows.h>
#include <process.h>
#include <iostream>
#include <atomic>

int Money = 0;

CRITICAL_SECTION MoneySection1;
CRITICAL_SECTION MoneySection2;

unsigned ThreadA(void* Arg)
{
	for (int i = 0; i < 1000000; ++i)
	{
		EnterCriticalSection(&MoneySection1);
		EnterCriticalSection(&MoneySection2);
		Money+= 10;
		LeaveCriticalSection(&MoneySection2);
		LeaveCriticalSection(&MoneySection1);
	}
	
	return 0;
}

unsigned ThreadB(void* Arg)
{
	for (int i = 0; i < 1000000; ++i)
	{
		EnterCriticalSection(&MoneySection1);
		EnterCriticalSection(&MoneySection2);
		Money--;
		LeaveCriticalSection(&MoneySection2);
		LeaveCriticalSection(&MoneySection1);
	}

	return 0;
}

int main()
{
	InitializeCriticalSection(&MoneySection1);
	InitializeCriticalSection(&MoneySection2);

	HANDLE ThreadHandle[2];

	ThreadHandle[0] = (HANDLE)_beginthreadex(
		nullptr,
		0,
		ThreadA,
		0,
		0,
		NULL);
	
	ThreadHandle[1] = (HANDLE)_beginthreadex(
		nullptr,
		0,
		ThreadB,
		0,
		0,
		NULL);

	WaitForMultipleObjects(2, ThreadHandle, true, INFINITE);
	DeleteCriticalSection(&MoneySection1);
	DeleteCriticalSection(&MoneySection2);

	printf("%d", (int)Money);
}