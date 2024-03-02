/**
 AUTHOR: Hitarth Thanki (hmthanki@uh.com)
 DATE: 2024-02-06
 COURSE: COSC 3360: Operating Systems
 PSID: 2131201
 PROFESSOR: Dr. Paris
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <iomanip>
using namespace std;

struct process
{
  int PID;
  double time;
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
  string state; // actual states are defined on line 62
};
class Scheduler
{
public:
  // global vars:
  queue<process> readyQueue, ssdQueue; // regular queues for the resp resources
  priority_queue<process> mainQueue;   // what the scheduler works with
  bool cpuIsEmpty = true, ssdIsEmpty = true;
  int buffer = 0, BSIZE; // bucket to keep track of buffer size across the scheduler.
  double clockTime;
  vector<input_tuple> inputTable;
  vector<ptable_tuple> processTable;
  const string READY = "READY", RUNNING = "RUNNING", TERMINATED = "TERMINATED", BLOCKED = "BLOCKED", UNINITIALIZED = "UNINITIALIZED";
  // ^ spaghetti coding enums, since c++ has a ghastly enum syntax.

  // methods:
  void readInput();
  void makeProcessTable();
  void initializeMainQueue();
  void arrivalFunction(process &);
  void completionFunction(process &);

  void requestCoreTime(process &);
  void requestSSDTime(process &);
  void requestIOTime(process &);

  void coreCompletion(process &);
  void ssdCompletion(process &);
  void iocompletion(process &);

  void terminateProcess(process &);
};
void Scheduler::readInput() // here purely for debugging. will cahnge back to main once bugs are fixed.
{
  ifstream in_file("/home/brohudev/treasurechest/00_College/02_Sophomore_Year_Archive/spring/OS (cosc 3360)/hw/hw1/input12.txt"); // todo fix this to use input redirection.

  if (!in_file.is_open())
  {
    cerr << "Error: Unable to open file." << endl;
  }

  string line;
  while (getline(in_file, line))
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

    inputTable.push_back(row);
  }
  in_file.close();
}
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
        processTable.push_back({num_process, 1, 1, 1, UNINITIALIZED});
        num_process++;
      }
    }
    else
    {
      if (inputLine.command == "START")
      {
        processTable[num_process - 1].endLine = i - 1;
        processTable.push_back({num_process, i, i, i, UNINITIALIZED});
        num_process++;
      }
      else if (i == inputTable.size() - 1)
      {
        processTable[num_process - 1].endLine = i;
      }
    }
  }

  // std::cout << "Line#  Operation\n\n";
  // for (auto &i : inputTable)
  // {
  //   int space = 10;
  //   int left = space - i.command.size();
  //   std::cout << i.command << std::string(left, ' ') << i.time << std::endl;
  // }
}

void Scheduler::initializeMainQueue()
{
  for (int index = 0; index < processTable.size(); index++)
  {
    process temp(processTable[index].pid, (inputTable[processTable[index].startLine].time), inputTable[processTable[index].startLine].command, 0, 0, 0);

    mainQueue.push(temp); // in it goes!
  }
}
void Scheduler::arrivalFunction(process &proc)
{
  processTable[proc.PID].currentLine = processTable[proc.PID].startLine + 1;
  requestCoreTime(proc); // the only command that runs on arrival of a new process.
};

void Scheduler::requestCoreTime(process &proc)
{
  if (cpuIsEmpty)
  {
    cpuIsEmpty = false;
    processTable[proc.PID].state = RUNNING;
    proc.time = clockTime + inputTable[processTable[proc.PID].currentLine].time;
    mainQueue.push(proc);
  }
  else
  {
    processTable[proc.PID].state = READY;
    readyQueue.push(proc);
  }
}
void Scheduler::requestSSDTime(process &proc)
{
  if (inputTable[processTable[proc.PID].currentLine].command == "READ")
  {
    if (buffer - inputTable[processTable[proc.PID].currentLine].time >= 0)
    {
      buffer -= inputTable[processTable[proc.PID].currentLine].time;
      proc.logicalReads++;
      mainQueue.push(proc);
    }
    else // physical read time
    {
      if (ssdIsEmpty) // if the ssd is empty push it onto the mainqueue.
      {
        ssdIsEmpty = false;
        buffer = buffer - inputTable[processTable[proc.PID].currentLine].time + BSIZE; // do the read
        proc.physicalReads++;                                                          // for the statuses
        proc.time += 0.1;
        mainQueue.push(proc);
      }
      else
      {
        ssdQueue.push(proc);
        processTable[proc.PID].state = BLOCKED; // it is still blocked if its waiting on the system to resolve other procs' requests.
      }
    }
  }
  if (inputTable[processTable[proc.PID].currentLine].command == "WRITE")
  {
    if (ssdIsEmpty) // we have the mansion to ourselves
    {
      ssdIsEmpty = false;
      proc.physicalWrites++;
      proc.time += 0.1;

      mainQueue.push(proc);
      processTable[proc.PID].state = BLOCKED; // all writess are blocking by default
    }
    else // stand it in the queue.
    {
      ssdQueue.push(proc);
      processTable[proc.PID].state = BLOCKED; // it is still waiting for another call to finish.
    }
  }
}
void Scheduler::requestIOTime(process &proc)
{
  proc.time = clockTime + inputTable[processTable[proc.PID].currentLine].time;
  mainQueue.push(proc); // it never has to wait for anything or do anything.
}

void Scheduler::terminateProcess(process &proc)
{
  processTable[proc.PID].state = TERMINATED;
  cout << "Process " << proc.PID << " terminates at time " << setprecision(1) << fixed << proc.time << " ms.\n";
  cout << "it performed " << proc.physicalReads << " physical read(s), " << proc.logicalReads << " in-memory read(s), ";
  cout << "and " << proc.physicalWrites << " physical write(s). \n";
  cout << "Process Table: \n";
  for (auto element : processTable)
    if (element.state != "-1") // this is so that terminated only prints once. spgehtti? yes. works? also yes :D
      cout << "Process " << element.pid << " is " << element.state << "." << endl;
  cout << endl;
  processTable[proc.PID].state = "-1"; // mark it to never show up in any other status "prints".
}
void Scheduler::coreCompletion(process &proc)
{
  cpuIsEmpty = true;
  if (!readyQueue.empty()) // let the process in the queue have its cpu time
  {
    process top = readyQueue.front();
    readyQueue.pop();
    requestCoreTime(top);
  }

  processTable[proc.PID].currentLine++;

  if (processTable[proc.PID].endLine < processTable[proc.PID].currentLine) // process go through all its instructions?
    terminateProcess(proc);
  else if (inputTable[processTable[proc.PID].currentLine].command == "INPUT" || inputTable[processTable[proc.PID].currentLine].command == "DISPLAY")
    requestIOTime(proc);
  else if (inputTable[processTable[proc.PID].currentLine].command == "READ" || inputTable[processTable[proc.PID].currentLine].command == "WRITE")
    requestSSDTime(proc);
};
void Scheduler::ssdCompletion(process &proc)
{
  ssdIsEmpty = true;
  if (!ssdQueue.empty()) // let the process in the ssd have its ssd time.
  {
    process top = ssdQueue.front();
    ssdQueue.push(proc);
    requestSSDTime(top);
  }

  processTable[proc.PID].currentLine++;
  processTable[proc.PID].state = READY;
  if (processTable[proc.PID].endLine < processTable[proc.PID].currentLine) // check if the ssd is the last command just in case
    terminateProcess(proc);
  else // the only command after an ssd completes is a core.
    requestCoreTime(proc);
}
void Scheduler::completionFunction(process &proc)
{
  if (inputTable[processTable[proc.PID].currentLine].command == "CORE")
    coreCompletion(proc);
  else if (inputTable[processTable[proc.PID].currentLine].command == "READ" || inputTable[processTable[proc.PID].currentLine].command == "WRITE")
    ssdCompletion(proc);
  else if (inputTable[processTable[proc.PID].currentLine].command == "INPUT" || inputTable[processTable[proc.PID].currentLine].command == "DISPLAY")
  {
    processTable[proc.PID].currentLine++;
    requestCoreTime(proc);
  }
}

int main()
{
  Scheduler scheduler;
  string line;
  while (getline(cin, line)) // read in the input file into a struct vector for ease of use
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
  scheduler.makeProcessTable();
  scheduler.initializeMainQueue();
  while (!scheduler.mainQueue.empty())
  {
    process top = scheduler.mainQueue.top();
    scheduler.mainQueue.pop();

    scheduler.clockTime = top.time; // jump the times
    if (scheduler.inputTable[scheduler.processTable[top.PID].currentLine].command == "START")
      scheduler.arrivalFunction(top);
    else
      scheduler.completionFunction(top);
  }
  return 0;
}