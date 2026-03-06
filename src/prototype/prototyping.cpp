#include <iostream>
#include <sqlite_orm/sqlite_orm.h>

#include <chrono>
#include <Windows.h>
#include <thread>
#include <queue>
#include <format>
#include "sqlite3.h"
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace sqlite_orm;
struct Time {
	int id;
	time_point<local_t, system_clock::duration> time;
};
std::queue<Time> times;
sqlite3* db;
struct Reminder {
	int id;
	std::string time;
	std::string status = "pending";
};
auto storage = make_storage("data.db", make_table("reminders",
	make_column("id", &Reminder::id, primary_key().autoincrement()),
	make_column("time", &Reminder::time),
	make_column("status", &Reminder::status, default_value("pending"))
));
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	auto local_zone_time = zoned_time{ current_zone(), system_clock::now() };

	/*if(!times.empty()){
		std::cout << times.front() << "\n";
		times.pop();
	}*/
	if (!times.empty()) {
		auto duration = duration_cast<seconds>(times.front().time - local_zone_time.get_local_time());
		std::cout << duration;
		if (duration < 5s && duration < 3s) {
			std::cout << "Time for alarm" << "\n";
			storage.update_all(set(c(&Reminder::status) = "done"), where(c(&Reminder::id) == times.front().id));
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
};

void main() {
	/*int err = sqlite3_open("data.db", &db);
	if (!err) {
		std::cout << "Database loaded!\n";
	}
	else {
		std::cout << "Error loading database!\n";
	}*/

	storage.sync_schema();
	auto local_zone_time = zoned_time{ current_zone(), system_clock::now() };

	//	auto t = local_zone_time.get_local_time() + 2h;
	auto t1 = local_zone_time.get_local_time() + 1min;
	auto t2 = local_zone_time.get_local_time() + 2min;
	//std::cout << t1;
	Reminder reminder1{ -1, std::vformat("{:%Y-%m-%d %H:%M:%S}", std::make_format_args(t1)) };
	Reminder reminder2{ -1, std::vformat("{:%Y-%m-%d %H:%M:%S}", std::make_format_args(t2)) };
	auto reminder1Id = storage.insert(reminder1);
	std::cout << reminder1Id << std::endl;
	reminder1.id = reminder1Id;
	auto reminder2Id = storage.insert(reminder2);
	std::cout << reminder2Id << std::endl;
	reminder2.id = reminder2Id;

	auto pending_alarms = storage.get_all<Reminder>(where(c(&Reminder::status) == "pending"));
	std::cout << pending_alarms.size() << std::endl;
	for (auto& alarm : pending_alarms) {
		std::cout << alarm.time << "," << alarm.status << std::endl;
		std::tm tm{};
		std::stringstream ss(alarm.time);
		ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
		auto tp = system_clock::from_time_t(std::mktime(&tm));
		auto local_time = zoned_time{ current_zone(), tp };
		times.push(Time{alarm.id, local_time.get_local_time() });
		std::cout << local_time << std::endl;
	}
	//times.push(local_zone_time.get_local_time() + 1min);
	//times.push(times.front() + 1min);
	//auto temp = times;
	/*while (!temp.empty()) {
		std::string s_sub = std::vformat("{:%Y-%m-%d %H:%M:%S}", std::make_format_args(temp.front()));
		Reminder reminder{ -1, s_sub };
		auto insertId = storage.insert(reminder);
		std::cout << "insertedId = " << insertId << std::endl;
		std::cout << "This needs to be saved! : " << s_sub << std::endl;
		temp.pop();
	}*/

	std::thread timerThread(testFunc);
	timerThread.detach();
	int f;
	std::cin >> f;
}
