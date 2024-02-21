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
  int arrivalTime, completionTime;
  string instruction;
  int logicalReads;
  int physicalReads;
  int physicalWrites;

  bool operator<(const process &other) const
  {
    return PID < other.PID;
  }
};

struct input_tuple
{
  std::string column1;
  int column2;
};

class Scheduler
{
public:
  queue<process> readyQueue, ssdQueue;
  priority_queue<process> mainQueue;
  bool cpuIsEmpty, ssdIsEmpty;
  int clockTime = 0, BSIZE;
  vector<input_tuple> inputTable;

  // put methods here.
  void readInput();
  void printInputTable();
};
void Scheduler::readInput()
{
  ifstream in_file("//hw/hw1/in.txt");

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

void Scheduler::printInputTable()
{
  std::cout << "Process Table:" << std::endl;
  std::cout << "--------------" << std::endl;
  std::cout << "Column 1\tColumn 2" << std::endl;
  std::cout << "--------------" << std::endl;
  for (const auto &row : inputTable)
  {
    std::cout << row.column1 << "\t\t" << row.column2 << std::endl;
  }
};

int main()
{
  Scheduler scheduler;

  scheduler.readInput();
  scheduler.printInputTable();
  // while (!scheduler.mainQueue.empty())
  // {
  //   process top = scheduler.mainQueue.top();
  //   scheduler.mainQueue.pop();
  // }
  return 0;
}