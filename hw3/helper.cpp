#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
using namespace std;

int money_total = 0;


void * one_thread (void*thread_info){
    
    int * cash_info;
    
    cash_info = (int *) thread_info;
    
    cout<<*cash_info<<endl;
    
    money_total+=*cash_info;
    
    
    return 0;
}
int main()
{
    //std::cout<<"Hello World";
    //need to create a thread ID first for each person, lets say I have 2 
    
    pthread_t tid[2];
    
    //Create a list of how much money they each have 
    
    int list_money[2] = {55,22};
    
    //create a single thread here 
    
    
    
    for (int i=0;i<2;i++){
        sleep(1); //remove those "//" to see how removing this will change the order of threads (they will race)
        
        //having no sleep the order can be either 55 or 22 first
        
        //having the sleep will make the order 55 then 22 (why we have a delay for the threads)
        
        pthread_create(&tid[i],nullptr,one_thread,(void*) &list_money[i]);
        
    
    }
    
    
    for (int i = 0; i < 2; i++) {
        pthread_join(tid[i], NULL);
    }
    
    //here we print the summary report if any 
    
    cout<<"Total Cash of People: "<<money_total<<endl;
    
    
    
    

    return 0;
}