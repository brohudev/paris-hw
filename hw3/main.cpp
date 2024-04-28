#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sstream>

int max_printing_processes;

void* process(void* arg) {
     char* myLine = strdup((char*)arg);
     char* name;
     char* arrival_delay;
     char* request;

     name = strtok(myLine, " ");
     arrival_delay = strtok(NULL, " ");
     request = strtok(NULL, " ");

     std::cout << "in child thread: Name: " << name << ", Arrival Delay: " << arrival_delay << ", Request: " << request << std::endl;

     return NULL;
}

int main(int argc, char* argv[]) {
     std::vector<pthread_t> threads;
     std::string line;
     while (std::getline(std::cin, line)) {
          std::cout << "main thread: " << line << std::endl; //! get rid of this
   
          sleep(1);
          threads.push_back(pthread_t());
          pthread_create(&threads.back(), NULL, process, (void*)line.c_str());
     }

     for (pthread_t& thread : threads) {
          pthread_join(thread, NULL);
     }

     return 0;
}