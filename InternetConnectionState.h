#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

enum class ConnectionType;
enum class ConnectionSpeed;
namespace Concurrency
{
	template <typename T>
	class task;
}

class InternetConnectionState
{
public:
	static ConnectionType GetInternetConnectionType();
	static bool _custom;
	static Concurrency::task<ConnectionSpeed> InternetConnectSocketAsync();
	static bool _connected;
	static ConnectionSpeed GetInternetConnectionSpeed(double);
	static Concurrency::task<double> RawSpeed();
};