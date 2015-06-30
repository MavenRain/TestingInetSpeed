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
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;
		struct addrinfo *result = nullptr;
		struct addrinfo *ptr = nullptr;
		struct addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		if (getaddrinfo("google.com", "8080", &hints, &result) != 0)
		{
			WSACleanup();
			--retries;
			return;
		}
		ptr = move(result);
		auto connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		freeaddrinfo(result);
		auto start = clock();
		if (SOCKET_ERROR == connect(connectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen)))
		{
			closesocket(connectSocket);
			WSACleanup();
			--retries;
			return;
		}
		if (connectSocket = INVALID_SOCKET)
		{
			closesocket(connectSocket);
			WSACleanup();
			--retries;
			return;
		}
		auto stop = clock();
		connectSpeeds.push_back((stop - start) / CLOCKS_PER_SEC * 1000);
		closesocket(connectSocket);
		WSACleanup();
	};

	vector<task<void>> connectTaskList;

	auto timeoutTask = create_task([task_timeout_ms]
	{
		sleep_for(milliseconds(task_timeout_ms));
	});

	for (auto i = 4; i > 0; --i)
	{
		connectTaskList.push_back(create_task(connectTask));
	}

	return (when_all(begin(connectTaskList), end(connectTaskList)) || timeoutTask).then([&]
	{
		//Compute speed...
		auto connectSpeedsSum = 0.0;
		//if (connectSpeeds.size() == 0) return ConnectionSpeed::Unknown;
		for (auto val : connectSpeeds)
		{
			connectSpeedsSum += val;
		}
		return GetInternetConnectionSpeed(connectSpeedsSum / connectSpeeds.size());		
	});
}




