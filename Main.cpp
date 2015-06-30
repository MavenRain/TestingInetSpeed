#include "InternetConnectionState.h"
#include <iostream>
#include "Enums.h"
#include <ppltasks.h>

using namespace std;

int main(int argc, char* argv[])
{
	while(true)
	{
		switch (InternetConnectionState::InternetConnectSocketAsync().get())
		{
		case ConnectionSpeed::High:
			cout << "High\r\n";
			break;
		case ConnectionSpeed::Average:
			cout << "Average\r\n";
			break;
		case ConnectionSpeed::Low:
			cout << "Low\r\n";
			break;
		default:
			cout << "Unknown\r\n";
			break;
		}
		cin.ignore();
	}
	
}