// Muhammad Rohaan Atique - 20I-0410
// Muhammad Usman Kamal - 20i-0562
// Kernel.cpp

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fstream>
#include "process.h"
#include "scheduler.cpp"
#include <sstream>
#include <queue>
#include <string>
using namespace std;
// Global Queues
queue<Process> readyQueue;

// Helper Function
int fileLineCounter(string fileName) {
    ifstream file;
    file.open(fileName);

    int lines = 0;
    string line;
    while(getline(file, line))
        lines++;

    return lines;
}


int main(int argc, char** argv) {

// Reading and storing arguments
if (argc < 4 || (argv[3][0] == 'r' && argc < 6)) {
    cout << "ERROR!! Invalid arguments specifiied. \n";
    cout << "[Syntax]: os-kernel <Input file> <# CPUs> r <timeslice> <Output file> \n";
    cout << "[Syntax]: os-kernel <Input file> <# of CPUs> f <Output file>\n";
    exit(0);
}

// Extract variables
string inputFile = argv[1];
int CPUCount = stoi(argv[2]);
char schedulerMethod = argv[3][0];
int timeSlice = -1;
string outputFile;
if (schedulerMethod != 'r' && schedulerMethod != 'f' && schedulerMethod != 'p' ) {
    cout << "ERROR!! Invalid scheduler method. \n";
    return -1;
}
if (CPUCount != 1 && CPUCount != 2 && CPUCount != 4) {
    cout << "ERROR!! Cpu Count can be 1, 2 or 4. \n";
    return -1;
}

if (argv[3][0] == 'r') {
    timeSlice = stoi(argv[4]);
    outputFile = argv[5];
} else outputFile = argv[4];
cout << "===============================================\n";
cout << "inputFile: " << inputFile << endl;
cout << "CPUCount: " << CPUCount << endl;
cout << "schedulerMethod: " << schedulerMethod << endl;
cout << "Time Slice: " << timeSlice << endl;
cout << "outputFile: " << outputFile << endl;
cout << "===============================================\n";
// Opening the process file
string fileName = "Processes1.txt";
int totalProcesses = fileLineCounter(fileName) - 1; //-1 to skip the 1st line
Process* pArr = new Process [totalProcesses];

ifstream procFile;
procFile.open(fileName, ios::in);

// Skipping the first line;
string line, word;
int procCounter = 1;
getline(procFile, line);

// Vars to use for initialization.

    int PID, Priority, IOTime;
    double arrivalTime, CPUTime;
    string PName;
    char procType;

while (getline(procFile, line)) {
    // cout << line << endl;
    stringstream ss;
    ss << line;

    ss >> PName;
    ss >> Priority;
    ss >> arrivalTime;
    ss >> procType; 
    ss >> CPUTime;
    ss >> IOTime;
    Process tmp (procCounter, PName, Priority, arrivalTime, procType, CPUTime, IOTime);
    pArr[procCounter-1] = tmp;

    procCounter++;
}
//printing the loaded data.
// for(int i = 0; i<totalProcesses-1; i++) {
//     pArr[i].printData();
// }

// Start scheduling
Scheduler S(CPUCount, schedulerMethod, pArr, totalProcesses, timeSlice);
S._start();
}
