/**
 TITLE: Assignment 3
 AUTHOR: Hitarth Thanki (hmthanki@uh.com)
 DATE: 2024-04-30
 COURSE: COSC 3360: Operating Systems
 PSID: 2131201
 PROFESSOR: Dr. Paris
 COMPILATION: none. default g++ build command should be enough to test using input redirection.
 */

#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sstream>

static int freeClerks;
static int numPatronsWaited;
static int numPatronsServed;

pthread_mutex_t clerkMutex;
pthread_cond_t clerkCond = PTHREAD_COND_INITIALIZER;

void* availService(void* line) {
     char* myLine = strdup((char*)line); //copy the parameter
     char* name = strtok(myLine, " "); //parse the values
     char* arrival_delay = strtok(NULL, " ");
     char* request = strtok(NULL, " ");

     pthread_mutex_lock(&clerkMutex);

          std::cout << name << " arrives at the post office." << std::endl;
          while (freeClerks <= 0) {// If no clerks are free, wait for a clerk to become free
               numPatronsWaited++;
               pthread_cond_wait(&clerkCond, &clerkMutex);
          }

          freeClerks--; // claim the clerk
          numPatronsServed++; 
          std::cout << name << " gets service." << std::endl;
     
     pthread_mutex_unlock(&clerkMutex);

     sleep(atoi(request));// Use the clerk's services

     pthread_mutex_lock(&clerkMutex);

          freeClerks++; // Release the clerk
          std::cout << name << " leaves the post office." << std::endl;

          pthread_cond_signal(&clerkCond); // Signal the next waiting thread

     pthread_mutex_unlock(&clerkMutex);

     pthread_exit((void*) 0); return NULL; //idk why I put both but they're there lol.
}

int main(int argc, char* argv[]) {
     std::vector<pthread_t> threads;
     std::vector<std::string> lines;
     std::string line;

     freeClerks = std::stoi(argv[1]);
     std::cout << "--- the post office has today " << freeClerks << " clerk(s) on duty." << std::endl;

     // Read all lines into a vector
     while (std::getline(std::cin, line)) {
          lines.push_back(line);
     }

     // Create threads in a separate loop
     for (int i = 0; i < lines.size(); i++) {
          const std::string& line = lines[i];

          size_t firstSpace = line.find(' '); size_t lastSpace = line.rfind(' ');
          std::string arrival_delay = line.substr(firstSpace + 1, lastSpace - firstSpace - 1);
          sleep(std::stoi(arrival_delay)); // sleep for the arrival_delay amount of time

          threads.push_back(pthread_t());//create thread
          pthread_create(&threads.back(), NULL, availService, (void*)line.c_str());
     }

     for (pthread_t& thread : threads) { //self-explanatory
          pthread_join(thread, NULL);
     }

     std::cout << "\nSUMMARY REPORT" << std::endl;
     std::cout << numPatronsServed << " patron(s) went to the post office" << std::endl;
     std::cout << numPatronsWaited << " patron(s) had to wait before getting service." << std::endl;
     std::cout << numPatronsServed - numPatronsWaited << " patron(s) did not have to wait." << std::endl;

     return 0;
}