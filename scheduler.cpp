// Muhammad Rohaan Atique - 20I-0410
// Muhammad Usman Kamal - 20i-0562
// scheduler.cpp

#include <iostream>
#include <pthread.h>
#include "process.h"
#include <queue>
#include <climits>
#include <semaphore.h>
#include <cstdlib>
#include <ctime>
#include <sys/wait.h>
#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <vector>
using namespace std;

sem_t newQueue_mutex, readyQueue_mutex, runningQueue_mutex, waitingQueue_mutex, terminatedQueue_mutex;
void* simulateCPU(void*);
void* simulateController(void*);
void wakeup(Process*, string);

//Global Vars
int completedProcesses = 0;
int totalProcesses = 0;
int global_cpuID = 0;
Process** currentProcess;

class Scheduler {
public:
    int cpuCount;
    char schedulerMethod;
    Process* pArr;
    int pArrCount;
    int timeSlice;    
    int total_contextSwitches = 0;
    queue<Process*> NEW;
    queue<Process*> READY;
    queue<Process*> RUNNING;
    queue<Process*> WAITING;
    queue<Process*> TERMINATED;
    vector<string> processesInWaiting;

    Scheduler() {

    }
    Scheduler(int CPUCount, char schedulerMethod, Process* pArr, int pArrCount, int timeSlice) {
        cpuCount = CPUCount;
        this -> schedulerMethod = schedulerMethod;
        this -> pArr = pArr;
        this -> pArrCount = pArrCount;
        this -> timeSlice = timeSlice;

        totalProcesses = pArrCount;
        //Load the processes into NEW.
        int remainingProcess = pArrCount;
        //load processes and set clock to 0
        int currentTime;
        while(remainingProcess > 0) {
            //Determine next process
            int lowestTime = INT_MAX;
            int prevI, chosenI = -1;
            for(int i =0; i<pArrCount; i++) {
                Process p = pArr[i];
                //p.printData();
                if (p.isExecuted == false && (p.arrivalTime < lowestTime)) {
                    lowestTime = p.arrivalTime;
                    chosenI = i;
                }
            }
            // Set executed status
            pArr[chosenI].isExecuted = true;
            cout << "chosenI: " << chosenI << "===";
            pArr[chosenI].printData();
            remainingProcess--; 
            
                //Put next process (sorted by arrival time)
                pArr[chosenI].PCB.state = "NEW";
                NEW.push(&pArr[chosenI]);
                wakeup(&pArr[chosenI], "READY");
            int currentArrivalTime = pArr[chosenI].arrivalTime;
            if (remainingProcess > 0) {
                // Find next process to load, and wait until its arrival time.
                int lowestTime = INT_MAX;
                int prevI, chosenI = -1;
                for(int i =0; i<pArrCount; i++) {
                    Process p = pArr[i];
                    //p.printData();
                    if (p.isExecuted == false && (p.arrivalTime < lowestTime)) {
                        lowestTime = p.arrivalTime;
                        chosenI = i;
                    }
                }
                int waitTime = pArr[chosenI].arrivalTime - currentArrivalTime;
                cout << "Waiting For Next Process: " << waitTime << endl;
                sleep(waitTime);
            }
        }
    }
    void setScheduler(int CPUCount, char schedulerMethod, Process* pArr, int pArrCount, int timeSlice) {
        cpuCount = CPUCount;
        this -> schedulerMethod = schedulerMethod;
        this -> pArr = pArr;
        this -> pArrCount = pArrCount;
        this -> timeSlice = timeSlice;
    }

    Scheduler copy() {
        Scheduler S (cpuCount, schedulerMethod, pArr, pArrCount, timeSlice);
        return S;
    }

//Starter function
void _start() {
    // Initializing time seed.
    srand(time(0));
    cout << "Ready Queue Size: " << READY.size() << endl;
    for(int i=0; i<pArrCount; i++) {
            Process p = pArr[i];
            p.printData();  
    }
    // Initialize currentArr
    currentProcess = new Process* [pArrCount];
    // Initialization of semaphores
    sem_init(&newQueue_mutex, 0, 1); //Binary semaphore
    sem_init(&readyQueue_mutex, 0, 1);
    sem_init(&runningQueue_mutex, 0, 1);
    sem_init(&waitingQueue_mutex, 0, 1);
    sem_init(&terminatedQueue_mutex, 0, 1);

    pthread_t threadCount[cpuCount];
    pthread_t replicationThread;
    // Thread for process execution
    for(int i=0; i<cpuCount; i++) {
        pthread_create(&threadCount[i], NULL, simulateCPU, this);
    }

	//sleep(100);
    // Thread for replication
    pthread_create(&replicationThread, NULL, simulateController, this);
    pthread_join(replicationThread, NULL);
}

char getSchedMethod() {
    return schedulerMethod;
}
// Load process from READY to RUNNING
void contextSwitch(Process* next) {
    total_contextSwitches++;
    if (next) {
        next -> PCB.state = "RUNNING";

        //Put to running
        sem_wait(&runningQueue_mutex);
        RUNNING.push(next);
        sem_post(&runningQueue_mutex);
    }
}
// Get next process from ready queue.
void schedule(int threadID) {
    
        sem_wait(&readyQueue_mutex);
        bool isEmpty = READY.empty();
        sem_post(&readyQueue_mutex);
    if (isEmpty) {
        Process* toRet = NULL;
        currentProcess[threadID] = toRet;
    }
    else {
        sem_wait(&readyQueue_mutex);
        Process* toRet = READY.front();
        READY.pop();
        sem_post(&readyQueue_mutex);
        contextSwitch(toRet);
        clock_t currentTime = clock();
        toRet -> timeInReady += (currentTime - toRet -> readyArrival) /CLOCKS_PER_SEC;
        currentProcess[threadID] = toRet;
        cout << "[schedule] ASSIGNED Process " << currentProcess[threadID] -> PCB.PName << " to [Thread " << threadID << "] \n";
    }
}
void wakeup(Process* P, string action) {
    if (action == "READY") {
        //Enqueue from NEW to READY
        Process* P = NEW.front();
        NEW.pop();
        P -> PCB.state = "READY";
        P -> readyArrival = clock();
        READY.push(P);
        cout << "[wakeup]: Pushed " << P -> PCB.PName << " to READY queue. \n";
    }
    else if (action == "WAITING") {
        P -> PCB.state = "READY";
        P -> readyArrival = clock();
        READY.push(P);
        cout << "[wakeup]: Pushed " << P -> PCB.PName << " from WAITING to READY queue. \n";
    }
    else 
        cout << "[ERROR]: wakeUp unknown argument. \n";
}

Process* idle() {
 if (READY.empty())
        return NULL;
    else {
        Process* toRet = READY.front();
        READY.pop();
        return toRet;
    }
}

void preempt(Process* P, int cputime) {
    P->CPUTime = cputime;
    READY.push(P);
    cout << "[wakeup]: Pushed " << P -> PCB.PName << " to READY queue. \n";
    
}

void terminate(int threadID) {
    completedProcesses++;
    //schedule(threadID);
}
void yield(int threadID) {
    //Mark process as waiting
    currentProcess[threadID] -> PCB.state = "WAITING";
    sem_wait(&waitingQueue_mutex);
    WAITING.push(currentProcess[threadID]);
    processesInWaiting.push_back(currentProcess[threadID] -> PCB.PName);
    sem_post(&waitingQueue_mutex);
    sleep(2);

    sem_wait(&waitingQueue_mutex);
    Process* P = WAITING.front();
    WAITING.pop();
    processesInWaiting.pop_back();
    sem_post(&waitingQueue_mutex);

    P -> IOTime--;
    wakeup(P, "WAITING");
}
};
void* simulateCPU(void* argsvp) {
        //load args
        Scheduler* sched = (Scheduler*) argsvp;
        sem_wait(&newQueue_mutex);
        int local_cpuID = global_cpuID++;
        sem_post(&newQueue_mutex);
        //cout << "[Thread " << local_cpuID << "] \n";
        char schedMethod = sched -> getSchedMethod();
        if (schedMethod == 'f') {
            //cout << "\n Chosen Approach: FCFS " << sched -> READY.size() << "\n";
            //Schedule is invoked when a process is ready to run a process
            // Loop back to here
            while(1) {
                sched -> schedule(local_cpuID);
                //If NULL, use the condition variable
                Process* p = currentProcess[local_cpuID];
                //cout << "NOW EXECUTING: "; p -> printData();
                    if ( p == NULL) {
                        //call idle
                        cout << "[Thread " << local_cpuID << "] ";
                        //Create Idle porcess
                        Process* idle_proc = new Process(-1, "IDLE", -1, 0, 'C', 0, -1);

                        currentProcess[local_cpuID] = idle_proc;
                        cout << sched -> READY.size();
                        cout << " - No Process in Ready Queue. \n";
                        sleep(10);
                    }
                    else {
                    //IF has IO, randomly go for IO n times (2s delay).
                    //Spend rest of time completing CPU Burst
                    //After completing expected time, call terminate(process)   
                        if (p->PCB.procType == 'I' && p -> IOTime > 0) {
                            cout << "IO Process \n";
                            cout << "[IO]:"; p->printData();
                            cout << "[IO][Thread " << local_cpuID << "] Beginning " << p->PCB.PName << "\n";
                            
                            int timeToGoForIO = (rand() % int(p -> CPUTime/2) + 1);
                            cout << "IO AT " << timeToGoForIO << endl;
                            //Execution
                            
                            sleep(p->CPUTime - timeToGoForIO);
                            p->CPUTime -= timeToGoForIO;
                            //wants to go for IO
                            sched -> yield(local_cpuID);

                        }
                        else {
                            cout << "[C][Thread " << local_cpuID << "]:  CPU Process \n"; p->printData();
                            cout << "[C][Thread " << local_cpuID << "] Beginning " << p->PCB.PName << "\n";
                            //Execute till CPU Burst.
                            sleep(p->CPUTime);
                            p->PCB.state = "TERMINATED";
                            cout << "[Thread " << local_cpuID << "]: Terminating Process " << p->PCB.PName << "\n";
                            sched -> terminate(local_cpuID);
                        }
                        //Remove from running
                        sched -> RUNNING.pop();
                    }
            }
            // Loop back

        }
        else if (schedMethod == 'r') {
            //Round robin algorithm
            while(1) 
            {
                sched -> schedule(local_cpuID); //schedule is invoked when a process is ready to run a process
                //If NULL, use the condition variable
                Process* p = currentProcess[local_cpuID];
                    if ( p == NULL) {
                        //idle
                        cout << "[Thread " << local_cpuID << "] ";
                        //Create Idle porcess
                        Process* idle_proc = new Process(-1, "IDLE", -1, 0, 'C', 0, -1);

                        currentProcess[local_cpuID] = idle_proc;
                        cout << sched -> READY.size();
                        cout << " - No Process in Ready Queue. \n";
                        sleep(10);
                    }
                    else 
                    {
                        //for checking  
                        cout << "Time slice  : " << sched->timeSlice << endl << "CPU TIme:  " << p->CPUTime << endl;  
                        if (p->PCB.procType == 'I' && p -> IOTime > 0) {
                            cout << "IO Process \n";
                            cout << "[IO]:"; p->printData();
                            cout << "[IO][Thread " << local_cpuID << "] Beginning " << p->PCB.PName << "\n";
                            
                            int timeToGoForIO = (rand() % int(p -> CPUTime/2) + 1);
                            cout << "IO AT " << timeToGoForIO << endl;
                            //Execution
                            
                            

                            sleep(2);
                            p->CPUTime -= 2;
                            
                            //wants to go for IO
                            sched -> yield(local_cpuID);
			     
                        }
                        
                        else 
                        {
                            cout << "[C][Thread " << local_cpuID << "]:  CPU Process \n"; p->printData();
                            cout << "[C][Thread " << local_cpuID << "] Beginning " << p->PCB.PName << "\n";
                            //Execute till CPU Burst.
                            
                            if (p->CPUTime < sched->timeSlice)
                            {
                            	cout << "Time slice  : " << sched->timeSlice << endl << "CPU TIme:  " << p->CPUTime << endl;  	
                            	sleep (p->CPUTime);
                            	p->PCB.state = "TERMINATED";
                            	cout << "[Thread " << local_cpuID << "]: Terminating Process " << p->PCB.PName << "\n";
                            	sched -> terminate(local_cpuID);
                            }
                            else
                            {
		                    cout << "Time slice  : " << sched->timeSlice << endl << "CPU TIme:  " << p->CPUTime << endl;  
		                    sleep (sched->timeSlice);
		                    p->CPUTime = p->CPUTime - sched->timeSlice;
		                    
		                    p->PCB.state = "PREMPTED";
		                    cout << "[Thread " << local_cpuID << "]: Premeptive Process " << p->PCB.PName << "\n";
		                    sched -> preempt(p,p->CPUTime);
                            }
                        }
                        
                        //Remove from running
                        sched -> RUNNING.pop();
                    }
            }

        }
}
void* simulateController(void* argsvp) {

cout << "THIS IS THE CONTROLLLLLERRRRR THREADDDDD***********************************************************************************************************************************************************" << endl;
        //load args
        Scheduler* sched = (Scheduler*) argsvp;
        
        //Starting GANTT
        cout << "Time     Ru   Re   Wa   ";
        for(int i=0; i<sched -> cpuCount; i++)
            cout << "CPU " << i << "\t\t";
        cout << "I/O Queue\n";
        //Printing ===
        cout << "======   ==   ==   ==\t";
        for(int i=0; i<sched -> cpuCount; i++)
            cout << "========\t";
        cout << "=============\n";

        double timer = 0.0;
        string cpuStr[sched -> cpuCount];
        while(completedProcesses != totalProcesses) {
            for(int i=0; i<sched -> cpuCount; i++) {
                cpuStr[i] = currentProcess[i] -> PCB.PName;
            }
            // string cpu0 = currentProcess[0] -> PCB.PName;
            // string cpu1 = currentProcess[1]->PCB.PName;
            // string cpu2 = currentProcess[2]->PCB.PName;
            // string cpu3 = currentProcess[3]->PCB.PName;

            //Read all processes from queue 
            stringstream ss; string IOProcesses = "";
            //Iterate over WAITING vector, 
            if (!sched -> processesInWaiting.empty()) {
                for(string s : sched -> processesInWaiting) {
                    ss << s << ",";
                }
                IOProcesses = ss.str();
            }
            
            if (sched -> cpuCount == 1) {
                printf("%f   %d   %d   %d    %s    <%s<\n", timer, sched -> RUNNING.size(), sched -> READY.size(), sched -> WAITING.size(), cpuStr[0].c_str(), IOProcesses.c_str());
            }
            else if (sched -> cpuCount == 2) {
                printf("%f   %d   %d   %d    %s    %s    <%s<\n", timer, sched -> RUNNING.size(), sched -> READY.size(), sched -> WAITING.size(), cpuStr[0].c_str(), cpuStr[1].c_str(), IOProcesses.c_str());
            }
            else if (sched -> cpuCount == 3) {
                printf("%f   %d   %d   %d    %s    %s    %s    %s    <%s<\n", timer, sched -> RUNNING.size(), sched -> READY.size(), sched -> WAITING.size(), cpuStr[0].c_str(), cpuStr[1].c_str(), cpuStr[2].c_str(), cpuStr[4].c_str(), IOProcesses.c_str());
            }
            // 100 ms sleep, 100000 because usleep takes in microseconds
            usleep(100000);
            timer += 0.1;
        }
        double timeTaken = timer; 
        // Print stats
        cout << "Total No. of Context Switches: " << sched -> total_contextSwitches << endl;
        cout << "Total Execution Time: " << timeTaken << "s \n";
        double totalTimeInReady = 0;
        for(int i=0; i<sched -> pArrCount; i++) {
            totalTimeInReady += sched -> pArr[i].timeInReady;
        }
        cout << "Total Time Spent in ready state: " << totalTimeInReady << " s \n\n";
    //After enqueueing, this thread will now print output until READ
}
