/**
 * @file hw1.cpp
 * @author Hitarth Thanki (hmthanki@uh.com)
 * @date 2024-02-06
 *
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
  int clockTime, BSIZE;
  vector<input_tuple> inputTable;
  vector<ptable_tuple> processTable;

  // methods:
  void readInput();        // stores the input file in `inputTable`
  void makeProcessTable(); // makes the process table used by the mainqueue
  void printProcessTable();
  void arrivalFunction(process &);
  void requestCoreTime(process &);
  void requestSSDTime(process &);
  void completion(process &);
  void terminateProcess(process &);
  void initializeMainQueue();
};

// standin for readvalues
void Scheduler::readInput()
{
  ifstream in_file("/home/brohudev/treasurechest/00_College/02_Sophomore_Year_Archive/spring/OS (cosc 3360)/hw/hw1/in.txt"); // todo fix this to use input redirection.

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
// standin for process_table
void Scheduler::makeProcessTable()
{
  int num_process = 0; // set as pid first then incremented to form the count
  for (int i = 0; i < inputTable.size(); ++i)
  {
    auto inputLine = inputTable[i];   // get line
    if (inputLine.command == "BSIZE") // if i = 0
    {
      BSIZE = inputLine.time;
    }
    else if (processTable.empty()) // first inputLine, at i = 0
    {
      if (inputLine.command == "START")
      {
        processTable.push_back({num_process, 1, 1, -1, 0});
        num_process++;
      }
    }
    else // rest of procs.
    {
      if (inputLine.command == "START") // new proc, and update last procs endline.
      {
        processTable[num_process - 1].endLine = i - 1;
        processTable.push_back({num_process, i, i, -1, 0});
        num_process++;
      }
      else if ((i + 1) == processTable.size())
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
// todo implement requestssdtime method
void Scheduler::requestSSDTime(process &proc)
{
  cout << "process requested ssd time." << endl;
}
void Scheduler::terminateProcess(process &proc)
{
  std::cout << "Process " << proc.PID << " terminates at t = " << proc.time << "ms. ";
  std::cout << "It performed " << proc.physicalReads << " physical read(s), " << proc.logicalReads << " logical read(s), ";
  std::cout << "and " << proc.physicalWrites << " physical write(s). ";
  std::cout << "Process states: ";
  std::cout << std::string(14, '-') << " ";
  // std::cout << PID << " TERMINATED " << PID + 1 << " RUNNING" << std::endl;
  // todo implement above line to print status of each process in processTable
}
/**
 * @brief This is where the cpu is freed, and where the instruction advacnes per process.
This is also the main loop that takes processes that are done with their time in the cpu and update them
 * @param proc
 */
void Scheduler::completion(process &proc)
{
  // declare the core to be open
  cpuIsEmpty = true;
  if (!readyQueue.empty()) // somebody in the queue before this process?
  {
    process top = readyQueue.front();
    readyQueue.pop();

    requestCoreTime(top); // the one in the queue goes first. then it gets added to the mainqueue.
  }

  if (processTable[proc.PID].endLine == processTable[proc.PID].currentLine) // process go through all its instructions?
  {
    terminateProcess(proc); // basically print it all out
  }
  else // well then let it move on to its next instruction.
  {
    processTable[proc.PID].currentLine++; // move  the currentLine along.
  }

  if (inputTable[processTable[proc.PID].currentLine].command == "READ" || inputTable[processTable[proc.PID].currentLine].command == "WRITE") // ssd request?
  {
    requestSSDTime(proc);
    return; // todo remove if needed
  }

  else if (inputTable[processTable[proc.PID].currentLine].command == "INPUT" || inputTable[processTable[proc.PID].currentLine].command == "DISPLAY")
  {
    proc.time += inputTable[processTable[proc.PID].currentLine].time; // get the time for which these two requests will run, and add it to the time of this proc.
    mainQueue.push(proc);                                             // back in it goes.
  }
};

int main()
{
  Scheduler scheduler;

  scheduler.readInput();
  scheduler.makeProcessTable();
  scheduler.initializeMainQueue();

  // the imp part:
  while (!scheduler.mainQueue.empty())
  {
    process top = scheduler.mainQueue.top();
    scheduler.mainQueue.pop();

    scheduler.clockTime = top.time; // set the clock time = time completion/arrival time
    if (scheduler.inputTable[scheduler.processTable[top.PID].currentLine].command == "START")
    {
      scheduler.arrivalFunction(top);
    }
    else
    {
      scheduler.completion(top); // the event is completion
    }
  }

  return 0;
}