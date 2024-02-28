/**
 AUTHOR: Hitarth Thanki (hmthanki@uh.com)
 DATE: 2024-02-06
 COURSE: COSC 3360: Operating Systems
 PSID: 2131201
 */
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
using namespace std;

struct process
{
  int PID;
  double time;
  // string instruction;
  int logicalReads;
  int physicalReads;
  int physicalWrites;

  process(int pid, double inputTime, const string instr,
          int logReads, int physReads, int physWrites)
      : PID(pid), time(inputTime),
        logicalReads(logReads), physicalReads(physReads),
        physicalWrites(physWrites) {}

  bool operator<(const process &other) const // so that the mainQueue correctly sorts using the time.
  {
    return other.time < time;
  }
};
struct input_tuple
{
  string command;
  int time;
};
struct ptable_tuple
{
  int pid;
  int startLine;
  int endLine;
  int currentLine;
  int state; // 1 = running, 2 = ready
};
class Scheduler
{
public:
  // global vars:
  queue<process> readyQueue, ssdQueue; // regular queues for the resp resources
  priority_queue<process> mainQueue;   // what the scheduler mainly works with
  bool cpuIsEmpty = true, ssdIsEmpty = true;
  int buffer = -1; // bucket to keep track of buffer size across the scheduler.
  int clockTime, BSIZE;
  vector<input_tuple> inputTable;
  vector<ptable_tuple> processTable;

  // methods:
  void makeProcessTable(); // makes the process table used by the mainqueue
  void printProcessTable();
  void initializeMainQueue();

  void arrivalFunction(process &);
  void completionFunction(process &);

  void requestCoreTime(process &);
  void requestSSDTime(process &);

  void coreCompletion(process &);
  void ssdCompletion(process &);
  void iocompletion(process &);

  void terminateProcess(process &);
};

// standin for process_table
void Scheduler::makeProcessTable()
{
  int num_process = 0; // set as pid first then incremented to form the count.
  for (int i = 0; i < inputTable.size(); ++i)
  {
    auto inputLine = inputTable[i];
    if (inputLine.command == "BSIZE")
    {
      BSIZE = inputLine.time;
    }
    else if (processTable.empty())
    {
      if (inputLine.command == "START")
      {
        processTable.push_back({num_process, 1, 1, -1, 0});
        num_process++;
      }
    }
    else
    {
      if (inputLine.command == "START")
      {
        processTable[num_process - 1].endLine = i - 1;
        processTable.push_back({num_process, i, i, -1, 0});
        num_process++;
      }
      else if (i == inputTable.size() - 1)
      {
        processTable[num_process - 1].endLine = i;
      }
    }
  }

  std::cout << "Line#  Operation\n\n";
  for (auto &i : inputTable)
  {
    int space = 10;
    int left = space - i.command.size();
    std::cout << i.command << std::string(left, ' ') << i.time << std::endl;
  }
}
void Scheduler::printProcessTable()
{
  cout << "PID\tStart Line\tEnd Line\tCurrent Line\tState\n";
  for (const auto &entry : processTable)
  {
    cout << entry.pid << "\t" << entry.startLine << "\t\t"
         << entry.endLine << "\t\t" << entry.currentLine << "\t\t"
         << entry.state << "\n";
  }
  cout << endl;
}
// create process structs using `processTable` and place them into `mainQueue`
void Scheduler::initializeMainQueue()
{
  // for all values in the p table
  for (int index = 0; index < processTable.size(); index++)
  {
    // create a struct with pid, its start time, its start command, and 0 for logical/physical reads/writes
    process temp(processTable[index].pid, (inputTable[processTable[index].startLine].time), inputTable[processTable[index].startLine].command, 0, 0, 0);

    mainQueue.push(temp); // in it goes!
  }
  // while (!mainQueue.empty()) // just here for debugging.
  // {
  //   process temp = mainQueue.top();
  //   mainQueue.pop();
  //   cout << temp.PID << ", ";
  //   cout << temp.instruction << ", ";
  //   cout << temp.time << ", ";
  //   cout << temp.logicalReads << ", ";
  //   cout << temp.physicalReads << ", ";
  //   cout << temp.physicalWrites << ", \n";
  // }
}
// entry point into the logic once scheduler detects the proc is a 'new' proc.
void Scheduler::arrivalFunction(process &proc)
{
  // change the currentline field of processtable to start line +1.
  processTable[proc.PID].currentLine = processTable[proc.PID].startLine + 1;

  // get command from input table (core. more like get the time for which it runs.)
  int command = inputTable[processTable[proc.PID].currentLine].time;

  cout << "Process# " << proc.PID << ", arrived with command: CORE " << command << endl;

  requestCoreTime(proc); // the only command that runs on arrival of a new process.
};

void Scheduler::requestCoreTime(process &proc)
{
  if (cpuIsEmpty)
  {
    // set global flag.
    cpuIsEmpty = false;
    // this means its running.
    processTable[proc.PID].state = 1;
    // get the time for the command at the current line of this proc and update it.
    // cout << "old proc time is: " << proc.time << endl; //? here purely for debugging
    proc.time += inputTable[processTable[proc.PID].currentLine].time;
    // proc.instruction = inputTable[processTable[proc.PID].currentLine].command; // todo remove this if needed.
    // cout << "new proc time is: " << proc.time << endl; //? purely for debugging

    mainQueue.push(proc); // push it back in.
  }
  else
  {
    processTable[proc.PID].state = 2; // which means the proc is in the readyQueue
    readyQueue.push(proc);            // chuck it in dere
    // cout << "pushed process#: " << proc.PID << "into readyQueue" << endl; //? here purely for debugging
  }
}
void Scheduler::requestSSDTime(process &proc)
{
  if (inputTable[processTable[proc.PID].currentLine].command == "READ") // is it a read command?
  {
    // todo look at paris's math on slide 71 for referrence on how to correctly perform a read.
    if (buffer - inputTable[processTable[proc.PID].currentLine].time >= 0) // can i do logical read?
    {
      buffer -= inputTable[processTable[proc.PID].currentLine].time; // do a logical read.
      proc.logicalReads++;                                           // mark the logical read
      cout << "process#: " << proc.PID << "performed a logical READ of: " << inputTable[processTable[proc.PID].currentLine].time << " Bytes" << endl;
      // todo get rid of this request if its incorrect.
      requestCoreTime(proc); // process next instruction in the process' store (corerequest.)
    }
    else // physical read time
    {
      buffer = buffer - inputTable[processTable[proc.PID].currentLine].time + BSIZE; // perform a physical read.
      proc.physicalReads++;                                                          // for the stats

      cout << "process #: " << proc.PID << " performed a logical READ at time " << proc.time << endl;

      if (ssdIsEmpty) // if the ssd is empty push it onto the mainqueue.
      {
        mainQueue.push(proc);             // there is no queue.
        processTable[proc.PID].state = 3; // set the state of this process as blocked, since its interrupted by an io call.
        cout << "process #: " << proc.PID << " got pushed into the mainqueue." << endl;
      }
      else // else push it into the ssdqueue.
      {
        ssdQueue.push(proc);
        processTable[proc.PID].state = 3; // it is still blocked if its waiting on the system who is doing calls for other procs.
        cout << "process #: " << proc.PID << " got pushed into the ssd queue with a blocked state." << endl;
      }
    }
  }
  if (inputTable[processTable[proc.PID].currentLine].command == "WRITE") // is it a write command?
  {
    proc.physicalWrites++; // trigger a "physical write"
    if (ssdIsEmpty)        // push the proc onto mainqueue if ssd is empty
    {
      mainQueue.push(proc);             // it is first in line.
      processTable[proc.PID].state = 3; // mark it is blocked because all writes are blocking.
      cout << "process #: " << proc.PID << " performed a physical WRITE. it is in the blocked state now." << endl;
    }
    else
    {
      ssdQueue.push(proc);
      processTable[proc.PID].state = 3; // mark it is blocked because it is still waiting for another call to finish.
      cout << "process #: " << proc.PID << " got pushed into the ssd queue, waiting to perform a WRITE." << endl;

    } // else push the proc onto ssd queue.
  }
}
void Scheduler::terminateProcess(process &proc)
{
  std::cout << "Process " << proc.PID << " terminates at t = " << proc.time << "ms. ";
  std::cout << "It performed " << proc.physicalReads << " physical read(s), " << proc.logicalReads << " logical read(s), ";
  std::cout << "and " << proc.physicalWrites << " physical write(s). ";
  std::cout << "Process states: ";
  std::cout << std::string(14, '-') << " ";
  cout << "PROCESS NO. \t STATUS \n";
  cout << "----------- \t ------ \n";
  for (auto element : processTable)
  {
    cout << element.pid << "\t" << element.state;
  }
}
/**
 * @brief This is where the cpu is freed, and where the instruction advacnes per process.
This is also the main loop that takes processes that are done with their time in the cpu and update them
 * @param proc
 */
void Scheduler::coreCompletion(process &proc)
{
  cpuIsEmpty = true;       // declare the core to be open
  if (!readyQueue.empty()) // somebody in the queue before this process?
  {
    process top = readyQueue.front();
    readyQueue.pop();
    requestCoreTime(top);  //? the one in the queue goes first. (this recurses until the queue is empty. idk if this is what i want)
    readyQueue.push(proc); // put the incoming proc in the queue.
    cout << "process #: " << proc.PID << " got pushed into the ready queue as it was not empty." << endl;
  }

  if (processTable[proc.PID].endLine == processTable[proc.PID].currentLine) // process go through all its instructions?
  {
    terminateProcess(proc); // print it all out
  }
  else // let it move on to its next instruction.
  {
    requestCoreTime(proc);                // let the proc have the core.
    processTable[proc.PID].currentLine++; // move  the currentLine along.
    cout << "process #: " << proc.PID << " completed its CORE request." << endl;
  }
};
void Scheduler::iocompletion(process &proc)
{

  proc.time += inputTable[processTable[proc.PID].currentLine].time; // update the times. .
  mainQueue.push(proc);                                             // back in it goes.
}
void Scheduler::ssdCompletion(process &proc)
{
  // declare the ssd to be open.
  ssdIsEmpty = true;
  if (!ssdQueue.empty()) // schedule the process thats waiting in ssdqueue if one
  {
    process top = ssdQueue.front();
    ssdQueue.push(proc); // let the proc stand in line.
    requestSSDTime(top); //
  }
  else // else increment the current line of the process by one.
  {
    requestSSDTime(proc); // let the proc have its time.

    if (inputTable[processTable[proc.PID].currentLine].command == "WRITE") // all writes take 0.1 ms.
      proc.time += 0.1;

    processTable[proc.PID].currentLine++; // OFFICIALLY MOVE ON TO THE NEXT LINE.
    cout << "process #: " << proc.PID << " just finished its ssd request and is back in the main queue." << endl;

    mainQueue.push(proc); // process its next instruction
  }
}
void Scheduler::completionFunction(process &proc) // pretty self explanatory.
{
  if (inputTable[processTable[proc.PID].currentLine].command == "CORE")
  {
    cout << "process #: " << proc.PID << " is here for a CORE completion event from mainq." << endl;

    coreCompletion(proc);
  }
  else if (inputTable[processTable[proc.PID].currentLine].command == "READ" || inputTable[processTable[proc.PID].currentLine].command == "WRITE")
  {
    cout << "process #: " << proc.PID << " is here for a SSD completion event from mainq." << endl;
    ssdCompletion(proc);
  }
  else if (inputTable[processTable[proc.PID].currentLine].command == "INPUT" || inputTable[processTable[proc.PID].currentLine].command == "DISPLAY")
  {
    cout << "process #: " << proc.PID << " is here for a INPUT/OUTPUT completion event from mainq." << endl;
    iocompletion(proc);
  }
}

int main()
{
  Scheduler scheduler;

  string line;
  while (getline(cin, line)) // read in the input file into a table for ease of use
  {
    line.erase(0, line.find_first_not_of(" \t"));
    line.erase(line.find_last_not_of(" \t") + 1);

    if (line.empty())
      break;

    istringstream iss(line);
    input_tuple row;

    if (!(iss >> row.command >> row.time))
    {
      cerr << "Error reading line: " << line << endl;
      continue;
    }

    scheduler.inputTable.push_back(row);
  }

  // build the rest of the data structures
  scheduler.makeProcessTable();
  scheduler.printProcessTable();
  scheduler.initializeMainQueue();

  // the imp part:
  // while (!scheduler.mainQueue.empty())
  // {
  //   process top = scheduler.mainQueue.top();
  //   scheduler.mainQueue.pop();

  //   scheduler.clockTime = top.time; // set the clock time = time completion/arrival time
  //   if (scheduler.inputTable[scheduler.processTable[top.PID].currentLine].command == "START")
  //   {
  //     scheduler.arrivalFunction(top);
  //   }
  //   else
  //   {
  //     scheduler.completionFunction(top); // the event is completion
  //   }
  // }

  return 0;
}