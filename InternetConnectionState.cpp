#include "InternetConnectionState.h"

#include "Enums.h"
#include <winsock2.h>
#include <WS2tcpip.h>
#include <time.h>
#include <vector>
#include <thread>
#include <chrono>
#include <ppltasks.h>
#include <array>

#pragma comment(lib, "Ws2_32.lib")



using namespace std;
using namespace this_thread;
using namespace chrono;
using namespace Concurrency;

ConnectionSpeed InternetConnectionState::GetInternetConnectionSpeed(double roundtriptime)
{
	if (!(roundtriptime > 0.0))
	{
		//RawSpeed = roundtriptime;
		return ConnectionSpeed::Unknown;
	}

	if (roundtriptime <= 0.0013)
	{
		//RawSpeed = roundtriptime;
		return ConnectionSpeed::High;
	}

	if (roundtriptime > 0.0013 && roundtriptime < 0.13)
	{
		//RawSpeed = roundtriptime;
		return ConnectionSpeed::Average;
	}

	//RawSpeed = roundtriptime;
	return ConnectionSpeed::Low;
}

task<ConnectionSpeed> InternetConnectionState::InternetConnectSocketAsync()
{
	auto retries = 4;
	long long task_timeout_ms = 1000;
	vector<double> connectSpeeds;

	auto connectTask = [&connectSpeeds, &retries, task_timeout_ms]
	{
		vector<char*> socketTcpWellKnownHostNames{ "google.com", "pandora.com", "facebook.com", "forbes.com" };
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return static_cast<double>(0);
		auto connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		auto host = gethostbyname("www.google.com");
		SOCKADDR_IN sockAddr;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(80);
		sockAddr.sin_addr.s_addr = *reinterpret_cast<unsigned long*>(host->h_addr);
		auto start = system_clock::now();
		if (0 != connect(connectSocket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr)) )
		{
			closesocket(connectSocket);
			WSACleanup();
			--retries;
			return static_cast<double>(0);
		}
		closesocket(connectSocket);
		WSACleanup();
		return static_cast<double>(duration_cast<milliseconds>(system_clock::now() - start).count());
	};

	vector<task<double>> connectTaskList;

	auto timeoutTask = create_task([task_timeout_ms]
	{
		sleep_for(milliseconds(task_timeout_ms));
		return static_cast<double>(0);
	});

	for (auto i = 4; i > 0; --i)
	{
		connectTaskList.push_back(create_task(connectTask));
	}

	return (when_all(begin(connectTaskList), end(connectTaskList)) || timeoutTask).then([&](vector<double> results)
	{
		//Compute speed...
		auto connectSpeedsSum = 0.0;
		//if (connectSpeeds.size() == 0) return ConnectionSpeed::Unknown;
		for (auto val : results)
		{
			connectSpeedsSum += val;
		}
		return GetInternetConnectionSpeed(connectSpeedsSum / connectSpeeds.size());		
	});
}




