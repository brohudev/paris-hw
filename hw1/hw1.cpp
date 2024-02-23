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
private:
  queue<process> readyQueue, ssdQueue;
  priority_queue<process> mainQueue;
  bool cpuIsEmpty, ssdIsEmpty;
  int clockTime = 0, BSIZE;
  vector<input_tuple> inputTable;
  vector<process> processTable;

public:
  // stores the input file in `inputTable`
  void readInput();
  void printInputTable();
  void makeProcessTable();
};

// standin for readvalues
void Scheduler::readInput()
{
  ifstream in_file("/home/brohudev/treasurechest/00_College/02_Sophomore_Year_Archive/spring/OS (cosc 3360)/hw/hw1/in.txt");

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
        processTable[num_process - 1].end = j - 1;
        processTable.push_back({num_process, i.column2, j});
        num_process++;
      }
      else if ((j + 1) == processTable.size())
      {
        processTable[num_process - 1].end = j;
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

void has_arrived(int Clock_Time, int PID)
{
  global emptyCPU;                                            // Assuming global variable declaration
  auto [command, value] = input_table[table_process[PID][4]]; // Assuming input_table and table_process are globally accessible
  if (emptyCPU)
  {
    emptyCPU = false;
    mainQueue.push_back({PID, "CORE", value + Clock_Time}); // Assuming mainQueue is a global queue
    table_process[PID][5] = "Running";
  }
  else
  {
    ReadyQueue.push({PID, value}); // Assuming ReadyQueue is a global queue
    table_process[PID][5] = "Ready";
  }
}
/*
 This conversion assumes the existence of global variables emptyCPU, input_table, table_process, mainQueue, and ReadyQueue, and the usage of structured bindings (auto [command, value]) to assign values from input_table[table_process[PID][4]]. Adjustments may be needed based on the actual implementation details of these global variables and queues. Also, remember that global variables are generally discouraged, and you might want to consider encapsulating them within a class or passing them as parameters instead.
*/
void finished_core(int Clock_Time, int PID)
{
  global emptyCPU; // Assuming global variable declaration
  if (!ReadyQueue.empty())
  {
    auto top = ReadyQueue.front();
    ReadyQueue.pop();
    mainQueue.push_back({top[0], "CORE", top[1] + Clock_Time}); // Assuming mainQueue is a global queue
    table_process[top[0]][5] = "Running";
  }
  else
  {
    emptyCPU = true;
  }

  // Go to the next request
  if (table_process[PID][3] >= table_process[PID][4] + 1)
  {
    table_process[PID][4] += 1;
    Request_Sort(Clock_Time, PID); // Assuming Request_Sort is a global function
  }
  else
  {
    completed(Clock_Time, PID); // Assuming completed is a global function
  }
}
// This conversion assumes the existence of global variables emptyCPU, ReadyQueue, mainQueue, and table_process, and the usage of front() and pop() for queue operations. Adjustments may be necessary based on the actual implementation details of these global variables and functions. Again, consider encapsulating global variables within a class or passing them as parameters instead.

void finished_sdd(int Clock_Time, int PID)
{
  global EmptySSD; // Assuming global variable declaration
  if (!SSDQueue.empty())
  {
    auto top = SSDQueue.front();
    SSDQueue.pop();
    mainQueue.push_back({top[0], "WRITE"}, 0.1 + Clock_Time); // Assuming mainQueue is a global queue
    table_process[top[0]][5] = "Blocked";
  }
  else
  {
    EmptySSD = true;
  }

  // Go to the next request
  if (table_process[PID][3] >= table_process[PID][4] + 1)
  {
    table_process[PID][4] += 1;
    Request_Sort(Clock_Time, PID); // Assuming Request_Sort is a global function
  }
  else
  {
    completed(Clock_Time, PID); // Assuming completed is a global function
  }
}
// As before, this conversion assumes the existence of global variables EmptySSD, SSDQueue, mainQueue, and table_process, and the usage of front() and pop() for queue operations. Adjustments may be necessary based on the actual implementation details of these global variables and functions. Consider encapsulating global variables within a class or passing them as parameters instead.

void completed(int Clock_Time, int PID)
{
  std::cout << "The process " << PID << " has terminated" << std::endl;
  table_process[PID][5] = "Terminated";
}

void Request_Sort(int Clock_Time, int PID)
{
  global EmptySSD;
  global emptyCPU;
  auto Request = input_table[table_process[PID][4]];
  if (Request[0] == "WRITE")
  {
    if (EmptySSD)
    {
      EmptySSD = false;
      mainQueue.push_back({PID, "WRITE"}, Clock_Time + 0.1);
      table_process[PID][5] = "Blocked";
    }
    else
    {
      SSDQueue.push({PID, Request[1]});
      table_process[PID][5] = "Blocked";
    }
  }
  else if (Request[0] == "CORE")
  {
    if (emptyCPU)
    {
      emptyCPU = false;
      mainQueue.push_back({PID, "CORE"}, Clock_Time + Request[1]);
      table_process[PID][5] = "Running";
    }
    else
    {
      ReadyQueue.push({PID, Request[1]});
      table_process[PID][5] = "Ready";
    }
  }
}
// Please note the following:

// I've assumed that EmptySSD, emptyCPU, input_table, table_process, mainQueue, SSDQueue, and ReadyQueue are global variables.
// In C++, std::cout is used for printing instead of print() in Python.
// The push operation for queues is push_back() for vectors (mainQueue, SSDQueue, ReadyQueue) and push() for queues (mainQueue, SSDQueue). Adjust accordingly based on the actual implementations.
// The syntax for accessing elements from vectors and queues in C++ is different from Python.

void print_visual(std::vector<std::vector<int>> &x, int clock, Queue &R, Queue &S, Queue &MQ, bool EC, bool ES)
{
  std::cout << "MainQueue : ";
  if (!MQ.isEmpty())
  {
    MQ.print_elements();
  }
  else
  {
    std::cout << "[]" << std::endl;
  }
  std::cout << std::endl;
  std::cout << "ReadyQueue : ";
  if (!R.isEmpty())
  {
    R.print_elements();
  }
  else
  {
    std::cout << "[]" << std::endl;
  }
  std::cout << std::endl;
  std::cout << "SSDQueue : ";
  if (!S.isEmpty())
  {
    S.print_elements();
  }
  else
  {
    std::cout << "[]" << std::endl;
  }
  std::cout << std::endl;

  std::vector<char> z;
  for (auto &i : x)
  {
    if (std::isalpha(i.back()))
    {
      z.push_back(i.back());
    }
  }
  for (int i = 0; i < z.size(); ++i)
  {
    std::cout << "Process " << i << " is " << z[i] << std::endl;
  }
  int count1 = (3 + (EC ? 1 : 3)) + 6 + (int)std::to_string(S.size()).length() - 2;
  int count2 = (3 + (EC ? 1 : 3)) + 6 + (int)std::to_string(R.size()).length() - 2;
  int maxie = std::max(count1, count2);

  if (count1 > count2)
  {
    std::cout << " CP: " << (EC ? "[A]>" : "[X]>") << " SQ: ";
    S.printR_elements();
    std::cout << std::endl;
    for (int i = 0; i < 2; ++i)
    {
      std::cout << " |" << std::string(maxie, ' ') << "|" << std::endl;
    }
    std::cout << " RQ: ";
    R.printR_elements();
    std::cout << "<" << std::string(count1 - count2, '-') << " SD: " << (ES ? "[A]" : "[X]") << std::endl;
  }
  else if (count1 < count2)
  {
    std::cout << " CP: " << (EC ? "[A]" : "[X]") << std::string(count2 - count1, '-') << "> SQ: ";
    S.printR_elements();
    std::cout << std::endl;
    for (int i = 0; i < 2; ++i)
    {
      std::cout << " |" << std::string(maxie, ' ') << "|" << std::endl;
    }
    std::cout << " RQ: ";
    R.printR_elements();
    std::cout << "< SD: " << (ES ? "[A]" : "[X]") << std::endl;
  }
  else
  {
    std::cout << " CP: " << (EC ? "[A]" : "[X]") << "> SQ: ";
    S.printR_elements();
    std::cout << std::endl;
    for (int i = 0; i < 2; ++i)
    {
      std::cout << " |" << std::string(maxie, ' ') << "|" << std::endl;
    }
    std::cout << " RQ: ";
    R.printR_elements();
    std::cout << "< SD: " << (ES ? "[A]" : "[X]") << std::endl;
  }
}
// In this C++ version:

// I've assumed Queue to be a class representing a queue.
// print_elements() and printR_elements() are assumed to be member functions of the Queue class that print the elements of the queue.
// Adjustments may be required based on the actual implementations of the queues and other data structures used in your program.

int main()
{
  Scheduler scheduler;

  scheduler.readInput();

  // while (!scheduler.mainQueue.empty())
  // {
  //   process top = scheduler.mainQueue.top();
  //   scheduler.mainQueue.pop();
  // }
  return 0;
}

// todo check this main method and incorporate as needed
/*int main() {
    // Create Tables
    process_table();
    readvalues();

    // Load all Arrival Times
    for (int i = 0; i < table_process.size(); ++i) {
        int Time = table_process[i].arrival_time;
        int ProcessID = table_process[i].num;
        mainQueue.push({Time, ProcessID});
    }

    int Clock_Time = 0;
    print_visual(Clock_Time, ReadyQueue, SSDQueue, mainQueue, emptyCPU, EmptySSD);

    while (!mainQueue.isEmpty()) {
        std::cout << std::string(130, '-') << std::endl;

        std::vector<int> element_popped = mainQueue.pop();
        int Time = element_popped[0];
        int ProcessID = element_popped[1];

        Clock_Time = Time;

        if (table_process[ProcessID].commands[table_process[ProcessID].current_index][0] == "START") {
            table_process[ProcessID].start_line = table_process[ProcessID].commands[table_process[ProcessID].current_index][1];
            table_process[ProcessID].status = "Arrived";
            ++table_process[ProcessID].current_index;
            has_arrived(Clock_Time, ProcessID);
        } else if (table_process[ProcessID].commands[table_process[ProcessID].current_index][0] == "CORE") {
            finished_core(Clock_Time, ProcessID);
        } else if (table_process[ProcessID].commands[table_process[ProcessID].current_index][0] == "WRITE") {
            finished_sdd(Clock_Time, ProcessID);
        }

        std::cout << std::endl;
        print_processt();
        std::cout << "The clock time is: " << Clock_Time << std::endl;
        print_visual(Clock_Time, ReadyQueue, SSDQueue, mainQueue, emptyCPU, EmptySSD);
    }

    return 0;
}*/
// This is a basic translation of the provided logic into C++. Make sure to fill in the implementations for process_table, readvalues, has_arrived, finished_core, finished_sdd, print_visual, and print_processt functions according to your requirements. Also, consider adjusting data structures and logic as needed for your specific application.