#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <assert.h>


#define NUM_CLIENTS 10


/* 
    Defining arguments of certaing functions with struct ,because pthread_create accepts only void *(void*) type of function.
*/

int inspected = 0;
int approved = 0;

// arguments for clients

typedef struct {
    int num_cones;
    int id;
} ClientArgs;


// arguments for manager

typedef struct {
    int total_cones;
} ManagerArgs;



// arguments for inspection of icecreams
typedef struct {
    sem_t lock; // semaphore for lock , because only one thread should touch certain part of code , initialized to 1
    sem_t requested; // semaphore for request from clerk to manager , initialized to 0 
    sem_t inspected; // semaphore from manager to clerk in order to give signal that ice cream has been inspected , initialized to 0
    int passed; // 0
    
} Inspection;


// arguments for queue of clients

typedef struct {
    sem_t lock; // 1
    int line_number; // 0
    sem_t requested; // 0
    sem_t customers[NUM_CLIENTS];

} Line;


// defining global variables for inspection and line
Inspection inspection;
Line line;



// generates random integer from 1 to 3

int RandomInteger() {
    srand(time(0));
    int num = rand() % 10 + 1;
    return num;
}

// returns 0 or 1 

int ZeroOrOne() {
    srand(time(0));  // Seed the random number generator
    int num = rand() % 2;
    return num;
}


// make icecream function

void MakeIceCream() {
    printf("Making ice cream\n");


}





void * Clerk(void * args) {
    sem_t done = *(sem_t*)args;
    
    int passed = 0;
    while(passed == 0) {
        MakeIceCream();
        sem_wait(&inspection.lock);
        sem_post(&inspection.requested);
        sem_wait(&inspection.inspected);
        passed = inspection.passed;
        sem_post(&inspection.lock);
    }





    sem_post(&done);
    return NULL;
}


void CheckOut() {
    printf("Checkout\n");
}

void * Cashier(void * args) {
    for(int i = 0;i<NUM_CLIENTS;i++) {
        sem_wait(&line.requested);
        CheckOut();
        sem_post(&line.customers[i]);

    }

    return NULL;
}

void * Client(void * args) {
    
    ClientArgs client = *(ClientArgs*)args;

    sem_t done;
    pthread_t x;

    sem_init(&done,0,0);

    for(int i = 0;i<client.num_cones;i++) {
        pthread_create(&x,NULL,Clerk,&done);
    }
    for(int i = 0;i<client.num_cones;i++) {
        sem_wait(&done);
    }

    sem_wait(&line.lock);
    int number  = line.line_number;
    line.line_number++;
    sem_post(&line.lock);
    sem_post(&line.requested);
    sem_wait(&line.customers[number]);

    return NULL;


}

void * Manager(void * args) {
    ManagerArgs x = *(ManagerArgs*)args;
    
    while(approved<x.total_cones) {
        sem_wait(&inspection.requested);
        inspection.passed = ZeroOrOne();
        if(inspection.passed==1) {
            approved++;
        }
        inspected++;

        sem_post(&inspection.inspected);

    }
    
    return NULL;
}


int main() {
    // initialize semaphores for inspection
    int all_cones = 0;
    sem_init(&inspection.lock,0,1);
    sem_init(&inspection.requested,0,0);
    sem_init(&inspection.inspected,0,0);
    inspection.passed = 0;

    // initialiazing semaphores for line

    sem_init(&line.lock,0,1);
    sem_init(&line.requested,0,0);
    line.line_number = 0;
    for(int i = 0;i<NUM_CLIENTS;i++) {
        sem_init(&line.customers[i],0,0);
    }

    ClientArgs clients[NUM_CLIENTS];

    // storing threads in array 
    pthread_t tid[NUM_CLIENTS];


    for(int i = 0;i<NUM_CLIENTS;i++) { 
        clients[i].id = i;
        clients[i].num_cones = RandomInteger();
        all_cones+=clients[i].num_cones;

        // creating threads for each client

        pthread_create(&tid[i],NULL,Client,&clients[i]);

    }

    pthread_t mag;
    pthread_t cash;

    ManagerArgs manager;
    manager.total_cones = all_cones;

    
    pthread_create(&mag,NULL,Manager,&manager);
    pthread_create(&cash,NULL,Cashier,NULL);


    // telling threads to wait untill all threads are done their work
    for(int i = 0;i<NUM_CLIENTS;i++) {
        pthread_join(tid[i],NULL);
    }
    pthread_join(mag,NULL);
    pthread_join(cash,NULL);

    printf("Inspected : %d and Approved : %d\n",inspected,approved);





    return 0;
}