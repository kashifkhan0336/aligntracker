#include <iostream>
#include <chrono>
#include <Windows.h>
#include <thread>
#include <queue>

using namespace std::chrono;
using namespace std::chrono_literals;
std::queue<time_point<local_t, system_clock::duration>> times;

void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	auto local_zone_time = zoned_time{ current_zone(), system_clock::now() };

	/*if(!times.empty()){
		std::cout << times.front() << "\n";
		times.pop();
	}*/
	if (!times.empty()) {
		auto duration = duration_cast<seconds>(times.front() - local_zone_time.get_local_time());
		std::cout << duration;
		if (duration < 5s && duration < 3s) {
			std::cout << "Time for alarm" << "\n";
			times.pop();
		}
	}

	//get the local time zone
	
	////extract day from current local time
	//time_point current_day = floor<days>(local_zone_time.get_local_time());
	////add 'time' to current day
	////auto alarm_time = current_day + hours(19) + minutes(42) + seconds(0);

	//std::cout << local_zone_time << "\n";
	//std::cout << alarm_time << "\n";
	//auto duration = duration_cast<seconds>(alarm_time - local_zone_time.get_local_time());
	//if (duration < 5s && duration > 3s) {
	//	std::cout << "Time left : " << duration_cast<seconds>(alarm_time - local_zone_time.get_local_time());
	//	std::cout << "Time for alarm!";
	//}
	//else {
	//	std::cout << "Time left : " << duration_cast<seconds>(alarm_time - local_zone_time.get_local_time());
	//	std::cout << "Not time for alarm!";
	//}
}
void testFunc() {

	std::cout << "Timer Thread started\n";
	SetTimer(NULL, 0, 1000, (TIMERPROC)TimerProc);
	MSG msg;
	// Main message loop
	while (GetMessage(&msg, NULL, 0, 0)) {
		// In this simple case, we are only interested in timer messages
		if (msg.message == WM_TIMER) {
			DispatchMessage(&msg); // This dispatches the message and calls TimerProc
		}
	}
	std::cout << "Thread over" << "\n";
}
void main() {
	auto local_zone_time = zoned_time{ current_zone(), system_clock::now() };

	//	auto t = local_zone_time.get_local_time() + 2h;
	
	times.push(local_zone_time.get_local_time() + 1min);
	times.push(times.front()+1min);
	std::thread timerThread(testFunc);
	timerThread.detach();
	int t;
	std::cin >> t;
}