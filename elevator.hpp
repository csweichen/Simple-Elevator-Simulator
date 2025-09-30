#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>

class Elevator {
	public:
		Elevator(int idx, std::mutex& mtx): 
			stdout_mtx(mtx),
			idx(idx),
			current_floor(1),
			running(true)
		{
			worker = std::thread(&Elevator::move, this);
		};

		Elevator(int idx, int cur_floor, std::mutex& mtx): 
			stdout_mtx(mtx),
			idx(idx),
			current_floor(cur_floor),
			running(true) 
		{
			worker = std::thread(&Elevator::move, this);
		};

		void display_floor() {
			{
				std::lock_guard<std::mutex> lock(stdout_mtx);
				std::cout << "[Elevator " << idx << "] floor " << current_floor << std::endl;
			}
		};

		int getCurrentFloor() {
			return current_floor.load();
		};

		void move() {
			while (running) {
				int target_floor = -1;
				{
					std::unique_lock<std::mutex> lock(queue_mtx);
					cv.wait(lock, [this]{ return !tasks.empty() || !running; });
					if (!running && tasks.empty()) break;
					target_floor = tasks.front();
					tasks.pop();
				}
				{
					std::lock_guard<std::mutex> lock(stdout_mtx);
					std::cout << "[Elevator " << idx << "] moving to floor " << target_floor << std::endl;
				}
				while (current_floor != target_floor) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					if (current_floor < target_floor) {
						current_floor++;
					} else if (current_floor > target_floor) {
						current_floor--;
					}
					{
						std::lock_guard<std::mutex> lock(stdout_mtx);
						std::cout << "[Elevator " << idx << "] [moving...] floor " << current_floor << std::endl;
					}
				}
				display_floor();
			}
			{
				// end elevator message
				std::lock_guard<std::mutex> lock(stdout_mtx);
				std::cout << "[Elevator " << idx << "] stopped." << std::endl;
			}
		};

		void addTask(int from_floor, int to_floor) {
			if (running) {
				{
					std::lock_guard<std::mutex> lock(queue_mtx);
					tasks.push(from_floor);
					tasks.push(to_floor);
				}
				cv.notify_all();
			}
		};

		void stop() {
			running = false;
			{
				std::lock_guard<std::mutex> lock(stdout_mtx);
				std::cout << "[Elevator " << idx << "] wait for stopping..." << std::endl;
			}
			cv.notify_all();
			if (worker.joinable()) {
				worker.join();
			}
		}

	private:
		int idx;
		std::atomic<int> current_floor;
		std::atomic<bool> running;
		std::thread worker;
		std::queue<int> tasks;
		std::mutex& stdout_mtx;
		std::mutex queue_mtx;
		std::condition_variable cv;
};
