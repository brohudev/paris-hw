// 1. Define a struct to hold patron data (name, arrival time, processing time).
// 2. Read the number of clerks from the command line.
// 3. Read the patron data from the standard input and store it in an array of structs.
// 4. Initialize a mutex and a condition variable.
// 5. Create a thread for each patron, passing the patron data to the thread function.
// 6. In the thread function, simulate the patron's behavior:
//    - Wait for the arrival time.
//    - Lock the mutex and wait for a free clerk (using the condition variable).
//    - Simulate the processing time.
//    - Unlock the mutex and signal the condition variable.
// 7. Join all threads.
// 8. Print the statistics.

// Here is the code:

#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <fstream>

struct Patron {
     std::string name;
     int arrival_time;
     int processing_time;
};

pthread_mutex_t mutex;
pthread_cond_t freeClerks;
int nFreeClerks, nPatrons, nWaited = 0;

void* patron(void *arg) {
     Patron* p = (Patron*) arg;
     sleep(p->arrival_time);
     std::cout << p->name << " arrives at the post office.\n";

     pthread_mutex_lock(&mutex);
     std::cout << "Patron data: " << "Name: " << p->name << ", Arrival Time: " << p->arrival_time << ", Processing Time: " << p->processing_time << "\n";
     pthread_mutex_unlock(&mutex);

     sleep(p->processing_time);

     pthread_mutex_lock(&mutex);
     std::cout << p->name << " leaves the post office.\n";
     pthread_cond_signal(&freeClerks);
     pthread_mutex_unlock(&mutex);

     pthread_exit((void*) 0);
}

int main(int argc, char *argv[]) {
     if (argc != 2) {
          std::cout << "Usage: " << argv[0] << " <number_of_clerks>\n";
          return 1;
     }

     nFreeClerks = std::stoi(argv[1]);
     pthread_mutex_init(&mutex, NULL);
     pthread_cond_init(&freeClerks, NULL);

     std::vector<Patron> patrons;
     std::vector<pthread_t> threads;
     Patron p;
     std::ifstream inputFile("input.txt"); // Assuming the input file is named "input.txt"
     while (inputFile >> p.name >> p.arrival_time >> p.processing_time) {
          patrons.push_back(p);
          threads.push_back(pthread_t());
          pthread_create(&threads.back(), NULL, patron, &patrons.back());
          nPatrons++;
     }
     inputFile.close();

     for (auto& t : threads)
          pthread_join(t, NULL);

     std::cout << "Total number of patrons serviced: " << nPatrons << "\n";
     std::cout << "Number of patrons that did not have to wait: " << nPatrons - nWaited << "\n";
     std::cout << "Number of patrons that had to wait: " << nWaited << "\n";

     return 0;
}