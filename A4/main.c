#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sched.h>



pthread_mutex_t rng_mutex,lane1_mutex,lane2_mutex,lane3_mutex,lane4_mutex,global_mutex;
int north=0,south=0,east=0,west=0;
int prevresolved = 1;


int thread_safe_rng(int min, int max) {
    pthread_mutex_lock(&rng_mutex);
    int r = rand();
    pthread_mutex_unlock(&rng_mutex);
    return min + r % max;
}

/* TODO : can add global vars, structs, functions etc */

void arriveLane(char* trainDir) {

    /* TODO: add code here */
    if(trainDir == "North"){
        pthread_mutex_lock(&global_mutex);
        pthread_mutex_lock(&lane1_mutex);
        north=1;
    }
    else if(trainDir == "South"){
        pthread_mutex_lock(&lane3_mutex);
        south=1;
    }
    else if(trainDir == "East"){
        pthread_mutex_lock(&lane4_mutex);
        east=1;
    }
    else{
        pthread_mutex_lock(&lane2_mutex);
        west=1;
    }
    
}

void crossLane(char* trainDir) {
    /* TODO: add code here */
    if(trainDir == "North"){
        pthread_mutex_lock(&lane2_mutex);
    }
    else if(trainDir == "South"){
        pthread_mutex_lock(&lane4_mutex);
    }
    else if(trainDir == "East"){
        pthread_mutex_lock(&lane1_mutex);
    }
    else{
        pthread_mutex_lock(&lane3_mutex);
    }
    usleep(1000 * thread_safe_rng(500, 1000)); // take 500-1000 ms to cross the lane
}

void exitLane(char* trainDir) {
    if(trainDir == "North"){
        north=0;
        pthread_mutex_unlock(&global_mutex);
        pthread_mutex_unlock(&lane1_mutex);
        pthread_mutex_unlock(&lane2_mutex);
    }
    else if(trainDir == "South"){
        south=0;
        pthread_mutex_unlock(&lane3_mutex);
        pthread_mutex_unlock(&lane4_mutex);
    }
    else if(trainDir == "East"){
        east=0;
        pthread_mutex_unlock(&lane4_mutex);
        pthread_mutex_unlock(&lane1_mutex);
    }
    else{
        west=0;
        pthread_mutex_unlock(&lane2_mutex);
        pthread_mutex_unlock(&lane3_mutex);
    }
    prevresolved = 1;
}



void* trainThreadFunction(void* arg)
{
    /* TODO extract arguments from the `void* arg` */
    usleep(thread_safe_rng(0, 10000)); // start at random time

    char* trainDir = NULL; // TODO set the direction of the train: North/South/East/West.
    if(*(char*)arg == 'N'){
        trainDir = "North";
    }
    else if(*(char*)arg == 'S'){
        trainDir = "South";
    }
    else if(*(char*)arg == 'E'){
        trainDir = "East";
    }
    else{
        trainDir = "West";
    }
    arriveLane(trainDir);
    printf("Train Arrived at the lane from the %s direction\n", trainDir);

    crossLane(trainDir);

    printf("Train Exited the lane from the %s direction\n", trainDir);
    exitLane(trainDir);
}

void* deadLockResolverThreadFunction(void * arg) {
    /* TODO extract arguments from the `void* arg` */
    
    while (1) {
        /* TODO add code to detect deadlock and resolve if any */
        int deadLockDetected = 0; // TODO set to 1 if deadlock is detected
        if((north == 1) && (south == 1) && (east == 1) && (west == 1) ){
            deadLockDetected = 1;
        }
        if (deadLockDetected && prevresolved) {
            printf("Deadlock detected. Resolving deadlock...\n");
            /* TODO add code to resolve deadlock */
            pthread_mutex_unlock(&lane1_mutex);
            prevresolved = 0;
            sched_yield();
        }

        usleep(1000 * 500); // sleep for 500 ms
    }
}




int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc != 2) {
        printf("Usage: ./main <train dirs: [NSWE]+>\n");
        return 1;
    }

    pthread_mutex_init(&rng_mutex, NULL);
    pthread_mutex_init(&lane1_mutex, NULL);
    pthread_mutex_init(&lane2_mutex, NULL);
    pthread_mutex_init(&lane3_mutex, NULL);
    pthread_mutex_init(&lane4_mutex, NULL);
    pthread_mutex_init(&global_mutex, NULL);


    /* TODO create a thread for deadLockResolverThreadFunction */
    pthread_t deadlock_t;
    pthread_create(&deadlock_t,NULL,deadLockResolverThreadFunction,NULL);

    char* train = argv[1];

    int num_trains = 0;
    
    pthread_t thread_ids[strlen(train)];
    while (train[num_trains] != '\0') {
        char trainDir = train[num_trains];

        if (trainDir != 'N' && trainDir != 'S' && trainDir != 'E' && trainDir != 'W') {
            printf("Invalid train direction: %c\n", trainDir);
            printf("Usage: ./main <train dirs: [NSEW]+>\n");
            return 1;
        }

        /* TODO create a thread for the train using trainThreadFunction */
        pthread_create(&thread_ids[num_trains], NULL, trainThreadFunction, &train[num_trains]);
        num_trains++;
    }

    /* TODO: join with all train threads*/
    for(int i = 0 ; i<num_trains; i++){
        pthread_join(thread_ids[i],NULL);
    }
    return 0;
}