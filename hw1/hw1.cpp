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
  string instruction;
  int logicalReads;
  int physicalReads;
  int physicalWrites;

  bool operator<(const process &other) const
  {
    return time < other.time; // todo idk what "time" the ppt is referring to. so this is a guess.
  }
};

struct input_tuple
{
  string column1;
  int column2;
};

struct ptable_tuple
{
  int pid;
  int startLine;
  int endLine;
  int currentLine;
  int state;
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

  void arrivalFunciton();
  void completion();
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

    if (!(iss >> row.column1 >> row.column2))
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
  int num_process = 0;
  for (int j = 0; j < inputTable.size(); ++j)
  {
    auto i = inputTable[j];
    if (i.column1 == "BSIZE")
    {
      BSIZE = i.column2;
    }
    if (processTable.empty())
    {
      if (i.column1 == "START")
      {
        processTable.push_back({num_process, i.column2, j});
        num_process++;
      }
    }
    else
    {
      if (i.column1 == "START")
      {
        processTable[num_process - 1].endLine = j - 1;
        processTable.push_back({num_process, i.column2, j});
        num_process++;
      }
      else if ((j + 1) == processTable.size())
      {
        processTable[num_process - 1].endLine = j;
      }
    }
  }
  std::cout << "Line#  Operation\n\n";
  for (auto &i : inputTable)
  {
    int space = 10;
    int left = space - i.column1.size();
    std::cout << i.column1 << std::string(left, ' ') << i.column2 << std::endl;
  }
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

int main()
{
  Scheduler scheduler;

  scheduler.readInput();
  scheduler.makeProcessTable();
  scheduler.printProcessTable();

  // the imp part:
  while (!scheduler.mainQueue.empty())
  {
    process top = scheduler.mainQueue.top();
    scheduler.mainQueue.pop();

    scheduler.clockTime = top.time; // set the clock time = time completion/arrival time
    if (top.instruction == "START")
      scheduler.arrivalFunction();
    else
      scheduler.completion();
  }
  return 0;
}
