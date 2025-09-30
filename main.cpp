#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <sstream>
#include <mutex>
#include "elevator.hpp"

std::mutex stdout_mtx;

int main() {
    Elevator elevator1(1, stdout_mtx);
    Elevator elevator2(2, stdout_mtx);

    std::string line = "";
    while (1) {
        {
            std::lock_guard<std::mutex> lock(stdout_mtx);
            std::cout << ">>";
        }
        std::getline(std::cin, line);
        std::stringstream ss(line);
        std::string command;
        ss >> command;
        if (command == "call") {
            // run the elevator

            // check floor number
            int from_floor, to_floor;
            ss >> from_floor >> to_floor;
            if (from_floor<1 || from_floor>10 || to_floor<1 || to_floor>10) {
                {
                    std::lock_guard<std::mutex> lock(stdout_mtx);
                    std::cout << "floor number error." << std::endl;
                    std::cout << "The floor number should be between 1 and 10." << std::endl;
                }
                continue;
            }
            // decide the elevator idx
            int e1_f = elevator1.getCurrentFloor();
            int e2_f = elevator2.getCurrentFloor();
            auto e = (abs(e1_f-from_floor)<abs(e2_f-from_floor)) ? &elevator1 : &elevator2;
            e->addTask(from_floor, to_floor);

        }
        else if (command == "status") {
            // show the floor of each elevator
            {
                std::lock_guard<std::mutex> lock(stdout_mtx);
                std::cout << "Elevator 1 is at floor " << elevator1.getCurrentFloor() << std::endl;
                std::cout << "Elevator 2 is at floor " << elevator2.getCurrentFloor() << std::endl; 
            }
        }
        else if (command == "exit") {
            // stop the program
            {
                std::lock_guard<std::mutex> lock(stdout_mtx);
                std::cout << "Wait for stopping the elevators..." << std::endl;
            }
            elevator1.stop();
            elevator2.stop();
            {
                std::lock_guard<std::mutex> lock(stdout_mtx);
                std::cout << "All elevators stopped." << std::endl;
            }
            break;
        }
        else {
            // error, help message
            {
                std::lock_guard<std::mutex> lock(stdout_mtx);
                std::cout << "unknown command" << std::endl;
                std::cout << "Please enter the following commands to control the elevator." << std::endl;
                std::cout << "call <current floor> <destination floor>" << std::endl;
                std::cout << "status" << std::endl;
                std::cout << "exit" << std::endl;
            }
        }

    }
    return 0;
}

