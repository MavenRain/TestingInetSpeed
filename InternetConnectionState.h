#pragma once

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
};