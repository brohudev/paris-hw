#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;
/*
cores: 2
BSIZE is the size of the hunk of memory. 
CORE: request that many ms of cpu time. 
READ: read that many BLOCKs from ssd, each one being 0.1 ms, regardless of read req. 
        the difference between block and read is what goes in the buffer
        asume buffer reads are instantaneous
DISPLAY: write to display for # ms
        basically do nothing?
INPUT: basically wait for 900ms
        basically do nothing?
WRITE: perform writes of size BSIZE until everything is written.
        every write takes 0.1 ms

START: start a new process at the # time stamp. 
output format each time a process finishes:
Process 0 terminates at t = 310ms.
It performed 0 physical read(s), 0 logical reads, and
1 physical write(s).

Process states:
--------------
  0 TERMINATED
  1 RUNNING

The simulation scheduler
1.Find the next event to process by looking at:
✓ Core request completion times
✓ SSD request completion times
✓ INPUT and DISPLAY request completion time
✓ Arrival time of the next process
2.Set current time to that time
3.Process event routine
4.Repeat until all processes are done

*/
int main(){
    cout << "hello world" <<endl;

    return 0;
}