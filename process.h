// Muhammad Rohaan Atique - 20I-0410
// Muhammad Usman Kamal - 20i-0562
// process.h

#pragma once
#include <iostream>
using namespace std;

struct PCBstruct {
    int PID;
    string PName;
    int Priority;
    char procType;
    string state;
};
class Process {
public:
    PCBstruct PCB;
    double arrivalTime;
    double CPUTime;
    int IOTime;
    bool isExecuted;
    clock_t readyArrival;
    double timeInReady;
    // loading all the process data
    Process() {
        //For array declaration;
    }
    Process (int PID, string PName, int Priority, double arrivalTime, char procType, double CPUTime, int IOTime) {
        this -> PCB.PID = PID;
        this -> PCB.PName = PName;
        this -> PCB.Priority = Priority;
        this -> PCB.procType = procType;
        PCB.state = "NEW";


        this -> arrivalTime = arrivalTime;
        this -> CPUTime = CPUTime;
        this -> IOTime = IOTime;
        this -> isExecuted = false;
        this -> timeInReady = 0;
    }

    void printData() {
        cout << PCB.PID << " " << PCB.PName << " " << PCB.Priority << " " << arrivalTime << " " << PCB.procType << " " << CPUTime << " " << IOTime << endl;
    }
};
