/*
    Shortest Remaining Time Next algorithm
    
    *** RmT : Remaining Time
    *** RT  : Running Time    
    
    - In the ready queue, processes are sorted based on their RmT
      So, a PQ is used to hold those processes with priority = RmT
    - Initially, Execution should start with that one at the front of PQ (min RmT)
    - While the process is being running, the scheduler should compare between
      the running process RmT and the RmT of the process at the front of the PQ
        - if the RmT process at the front of the PQ < running process RmT then switch process
    - To get the remaining time of the running process we can use shmadrr
    - Keep doing that while there still a process in the PQ

*/

#include "headers.h"

int main (int argc, char *argv[]) {

    bool fir = true;
    process running_process;

    while (true) {
        
        // PQ being empty doesn't mean that there are no processes to run e.g., depends on arrival time
        // it just a temporarily solution to be busy waiting
        while(PQ.empty()) ;

        if (fir) {
            running_process = PQ.front();
            PQ.pop();
            running_process.ProcessStart();
            fir = false;
        } else {
            if (running_process.RemainingTime) {
                if (running_process.RemainingTime > PQ.front().RunningTime) {
                    running_process.ProcessStop();
                    PQ.push(running_process);
                    running_process = PQ.front();
                    if (running_process.RemainingTime == running_process.RunningTime) {
                        running_process.ProcessStart();
                    } else {
                        running_process.ProcessResume();
                    }
                }
            } else {    // Process has finished
                running_process.ProcessTerminate();
            }
        }
    }
}

