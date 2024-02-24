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
  double completionTime, arrivalTime;
  string instruction;
  int logicalReads;
  int physicalReads;
  int physicalWrites;

  bool operator<(const process &other) const
  {
    return completionTime < other.completionTime; // todo idk what "time" the ppt is referring to. so this is a guess.
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

  bool cpuIsEmpty, ssdIsEmpty;
  int clockTime, BSIZE;

  vector<input_tuple> inputTable;
  vector<ptable_tuple> processTable;

  // methods:

  void readInput(); // stores the input file in `inputTable`

  void makeProcessTable(); // makes the process table used by the mainqueue
  void printProcessTable();

  void arrivalFunction();
  void completion();

  // create process structs using `processTable` and place them into `mainQueue`
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
  std::cout << "PID\tStart Line\tEnd Line\tCurrent Line\tState\n";
  for (const auto &entry : processTable)
  {
    std::cout << entry.pid << "\t" << entry.startLine << "\t\t"
              << entry.endLine << "\t\t" << entry.currentLine << "\t\t"
              << entry.state << "\n";
  }
}

void Scheduler::initializeMainQueue()
{
  // create a process struct
  // copy over all of the details from the process and input tables for this process.
  cout << "hello";
}

// todo implement these two methods:
void Scheduler::arrivalFunction(){};
void Scheduler::completion(){};

int main()
{
  Scheduler scheduler;

  scheduler.readInput();

  scheduler.makeProcessTable();

  scheduler.initializeMainQueue();

  scheduler.printProcessTable(); // just here for debugging

  // the imp part:
  // while (!scheduler.mainQueue.empty())
  // {
  //   process top = scheduler.mainQueue.top();
  //   scheduler.mainQueue.pop();

  //   scheduler.clockTime = top.completionTime; // set the clock time = time completion/arrival time
  //   if (top.instruction == "START")           // the event is a start
  //     scheduler.arrivalFunction();
  //   else
  //     scheduler.completion(); // the event is completion
  // }
  return 0;
}
