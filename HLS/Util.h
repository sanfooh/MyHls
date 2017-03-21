#pragma once
#include<Windows.h>
#include<stdio.h>
#include <time.h>
#include<string>
#include<chrono>
#include <map>
#include "GuidHelper.h"
#include <psapi.h>  
#include <sstream>
#include <TlHelp32.h>
#pragma comment(lib,"psapi.lib")  
using namespace std;


class ProcessStat
{
public:
	/// 时间转换  
	static uint64_t file_time_2_utc(const FILETIME* ftime)
	{
		LARGE_INTEGER li;

		assert(ftime);
		li.LowPart = ftime->dwLowDateTime;
		li.HighPart = ftime->dwHighDateTime;
		return li.QuadPart;
	}

	/// 获得CPU的核数  
	static int get_processor_number()
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return (int)info.dwNumberOfProcessors;
	}

	static int GetCpuUsage()
	{
		//cpu数量  
		static int processor_count_ = -1;
		//上一次的时间  
		static int64_t last_time_ = 0;
		static int64_t last_system_time_ = 0;


		FILETIME now;
		FILETIME creation_time;
		FILETIME exit_time;
		FILETIME kernel_time;
		FILETIME user_time;
		int64_t system_time;
		int64_t time;
		int64_t system_time_delta;
		int64_t time_delta;

		int cpu = -1;


		if (processor_count_ == -1)
		{
			processor_count_ = get_processor_number();
		}

		GetSystemTimeAsFileTime(&now);

		if (!GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time,
			&kernel_time, &user_time))
		{
			return -1;
		}
		system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time))

			/
			processor_count_;
		time = file_time_2_utc(&now);

		if ((last_system_time_ == 0) || (last_time_ == 0))
		{
			// First call, just set the last values.  
			last_system_time_ = system_time;
			last_time_ = time;
			return -1;
		}

		system_time_delta = system_time - last_system_time_;
		time_delta = time - last_time_;

		assert(time_delta != 0);

		if (time_delta == 0)
			return -1;

		// We add time_delta / 2 so the result is rounded.  
		cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
		last_system_time_ = system_time;
		last_time_ = time;
		return cpu;
	}

	static int GetMemoryUsage(uint64_t* mem, uint64_t* vmem)
	{
		PROCESS_MEMORY_COUNTERS pmc;
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		{
			if (mem) *mem = pmc.WorkingSetSize;
			if (vmem) *vmem = pmc.PagefileUsage;
			return 0;
		}
		return -1;
	}

	static int GetIoBytes(uint64_t* read_bytes, uint64_t* write_bytes)
	{
		IO_COUNTERS io_counter;
		if (GetProcessIoCounters(GetCurrentProcess(), &io_counter))
		{
			if (read_bytes) *read_bytes = io_counter.ReadTransferCount;
			if (write_bytes) *write_bytes = io_counter.WriteTransferCount;
			return 0;
		}
		return -1;
	}

	static int GetHandleCount()
	{
		DWORD dwHandles;
		GetProcessHandleCount(GetCurrentProcess(), &dwHandles);
		return dwHandles;
	}

	//copy from msdn
	static int GetThreadCount()
	{
		HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
		THREADENTRY32 te32;
		int i = 0;
		hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (hThreadSnap == INVALID_HANDLE_VALUE)
			return(FALSE);
		te32.dwSize = sizeof(THREADENTRY32);
		auto dwOwnerPID = GetCurrentProcessId();
		if (!Thread32First(hThreadSnap, &te32))
		{
			CloseHandle(hThreadSnap);
			return(FALSE);
		}
		do
		{
			if (te32.th32OwnerProcessID == dwOwnerPID)
			{
				i++;
			}
		} while (Thread32Next(hThreadSnap, &te32));
		CloseHandle(hThreadSnap);
		return i;
	}

};

class Util
{
public:
	/// <summary>
	/// Gets the process stat.
	/// </summary>
	/// <returns></returns>
	static string GetProcessStat(void)
	{
		stringstream ss;
		int cpu;
		uint64_t mem, vmem, r, w;


		cpu = ProcessStat::GetCpuUsage();
		ProcessStat::GetMemoryUsage(&mem, &vmem);
		ProcessStat::GetIoBytes(&r, &w);
		int hanler = ProcessStat::GetHandleCount();
		int thread = ProcessStat::GetThreadCount();

		ss << "CPU:" << cpu << "% " << "Mem(M):" << mem / 1000 / 1000 << " " << "Virtual Mem(M):" << vmem / 1000 / 1000 << " " << "Read(M):" << r / 1000 / 1000 << " "
			<< "Write(M):" << w / 1000 / 1000 << "  Handler:" << hanler << " Thread:" << thread;
		return ss.str();
	}

	/// <summary>
	/// Debugs the specified content.
	/// </summary>
	/// <param name="content">The content.</param>
	/// <param name="">The .</param>
	static void Debug(const char* content, ...)
	{
		char buf[200] = { 0 };
		va_list st;
		va_start(st, content);
		vsprintf(buf, content, st);
		va_end(st);

		auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		struct tm* ptm = localtime(&tt);
		char date[260] = { 0 };
		sprintf(date, "%d-%02d-%02d %02d:%02d:%02d  %s\n",
			(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
			(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec, buf);
		OutputDebugStringA(std::string(date).c_str());
	}

	/// <summary>
	/// Sleeps the ms.
	/// </summary>
	/// <param name="ms">The ms.</param>
	static void SleepMs(int ms)
	{
		std::this_thread::sleep_for(chrono::milliseconds(ms));
	}

	/// <summary>
	/// Replaces the specified string.
	/// </summary>
	/// <param name="str">The string.</param>
	/// <param name="old_value">The old_value.</param>
	/// <param name="new_value">The new_value.</param>
	/// <returns></returns>
	static string&   Replace(string&   str, const   string&   old_value, const   string&   new_value)
	{
		for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
			if ((pos = str.find(old_value, pos)) != string::npos)
				str.replace(pos, old_value.length(), new_value);
			else   break;
		}
		return   str;
	}

	static string GetGuid()
	{
		GuidHelper helper;
		return  helper.GetGuidNoHyphen();
	}

};


/// <summary>
/// Event for class destruct wait thread exit(windows)
/// </summary>
class CloseEvent
{
	HANDLE h;
	int second = 0;
public:
	bool isQuit = false;

	CloseEvent()
	{
		h = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	~CloseEvent()
	{
		CloseHandle(h);
	}

	void WaitClose()
	{
		isQuit = true;
		WaitForSingleObject(h, INFINITE);
	}
	void CloseNow()
	{
		SetEvent(h);
	}

};
