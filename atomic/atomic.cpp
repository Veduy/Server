#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <iostream>
#include <atomic>
#pragma comment(lib, "ws2_32")

int a = 10000;

class SpinLock
{
public:
	void Lock()
	{
		bool expected = false;
		while (!locked.compare_exchange_weak(expected, true, std::memory_order_acquire))
		{
			expected = false;
		}
	}

	void UnLock()
	{
		locked.store(false, std::memory_order_release);
	}

protected:
	std::atomic_bool(locked);

};

class SpinLock2
{
public:
	void Lock()
	{
		while (lockFlag.test_and_set(std::memory_order_acquire))
		{

		}
	}

	void UnLock()
	{
		lockFlag.clear(std::memory_order_release);
	}

protected:
	std::atomic_flag lockFlag = ATOMIC_FLAG_INIT;
};

SpinLock spinLock;
SpinLock2 spinLock2;
unsigned TempThread1(void* Arg)
{
	for(int i =0; i < 100000; ++i)
	{
		//spinLock.Lock();
		spinLock2.Lock();
		a++;
		spinLock2.UnLock();
		//spinLock.UnLock();
	}

	return 0;
}

unsigned TempThread2(void* Arg)
{
	for (int i = 0; i < 100000; ++i)
	{
		//spinLock.Lock();
		spinLock2.Lock();
		a--;
		spinLock2.UnLock();
		//spinLock.UnLock();
	}
	return 0;
}

int main(int argc, void* argv[])
{
	HANDLE Threads[2];
	
	Threads[0] = (HANDLE)_beginthreadex(nullptr, 0, TempThread1, 0, 0, 0);
	Threads[1] = (HANDLE)_beginthreadex(nullptr, 0, TempThread2, 0, 0, 0);

	WaitForMultipleObjects(2, Threads, true, INFINITE);

	std::cout << a << std::endl;
}