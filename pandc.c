#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>


int N; //number of buffers
int P; //number of producers
int C; //number of consumers
int X; //number of items each producer thread will create
int Ptime; //(seconds) sleep time for producers after item creation
int Ctime; //(seconds) sleep time for consumers after item consumption

int* buffer; //functions as shared resource between Producer and Consumer threads
int buffer_Position = 0; //to be modified by each Producer and entered into the Buffer
int* producer_Array;
int* consumer_Array;
int consumer_Array_Position = 0;
int producer_Array_Position = 0;
int number_to_Consume;
int item = 1;
int producer_sum = 0;
int consumer_sum = 0;
bool arrays_Matching = true;

//sem_t sem;
//sem_init(&s->sem,0,value); //initialize semaphore
//sem_wait(&s->sem);         //lock semaphore
//sem_post(&s->sem);         //unlock semaphore
//sem_destroy(&s->sem);

//pthread_create
//pthread_join
//pthread_exit
//pthread_t
//pthread_mutex_lock
//pthread_mutex_unlock
//PTHREAD_MUTEX_INITIALIZER
pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;
sem_t add;
sem_t delete;

struct thread_info{
    pthread_t tid;
    int readable_id;
    char* type;
    int consumer_num;
};

/**
 * for the 2 functions below, look
 * at the process slides/video for
 * sample code that can be used.
 */
/*
 * Function to remove item.
 * Item removed is returned
 */
int grab_item()
{
//    sem_wait(&delete);//must wait till given permission to delete
    int returnValue;
    buffer_Position--;
    returnValue = buffer[buffer_Position];
    consumer_Array[consumer_Array_Position] = returnValue;
    consumer_Array_Position++;
    consumer_sum = consumer_sum + returnValue;
    return returnValue;
}

/*
 * function to put item
 * into shared resource
 * so it can be consumed
 */
void put_item(int new_item)
{
    producer_sum = producer_sum + new_item;
    buffer[buffer_Position] = new_item;
    producer_Array[producer_Array_Position] = new_item;
    buffer_Position++;
    producer_Array_Position++;
}

void *producer_Thread(void* arg)
{
    struct thread_info* tinfo = (struct thread_info*)arg;
    //printf("Producer ID: %3d\n\n", tinfo->readable_id);
    for(int i = 0; i < X; i++) {
            sem_wait(&add);
            pthread_mutex_lock(&thread_lock);
            put_item(item);
            printf("%d\twas produced by Producer -->\t%d\n", item, tinfo->readable_id);
            item++;
            pthread_mutex_unlock(&thread_lock);
            sem_post(&delete);
            sleep(Ptime);
    }
    pthread_exit(0);
}

void *consumer_Thread(void* arg)
{
    struct thread_info* tinfo = (struct thread_info*)arg;
    //printf("Consumer ID: %3d\n\n", tinfo->readable_id);
    for(int i = 0; i < tinfo->consumer_num; i++) {
        sem_wait(&delete);
        pthread_mutex_lock(&thread_lock);
        if(buffer[buffer_Position-1]==0){
            i--;
        }else {
            int returnValue = grab_item();
            printf("%d\twas consumed by Consumer -->\t%d\n", returnValue, tinfo->readable_id);
        }
        pthread_mutex_unlock(&thread_lock);
        sem_post(&add);
        sleep(Ctime);
    }
    pthread_exit(0);
}

/**
 * insertionSort for sorting Consumer Array
 * @param arr
 * @param n
 */
void insertionSort(int arr[], int n)
{
    int i, key, j;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;

        /* Move elements of arr[0..i-1], that are
          greater than key, to one position ahead
          of their current position */
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}


int main(int argc, char* argv[])
{
    struct timespec start_time;
    struct timespec end_time;
    time_t seconds;
    long nano_seconds;
    int arraySize; //used to determine length of Producer/Consumer arrays for testing

    int value = 1;
    sem_init(&add,0,value);
    sem_init(&delete,0,value);

    /**
     * exit if incorrect number of args
     */
    if(argc != 7){
        printf("This program must be ran with the format: ./pandc N P C X Ptime Ctime \n"
               "N = Number of Buffers\n"
               "P = Number of Producers\n"
               "C = Number of Consumers\n"
               "X = Number of items each Producer will create\n"
               "Ptime = Sleep time for Producers after item creation\n"
               "Ctime = Sleep time for Consumers after item consumption\n");
        return -1;
    }

    /**
     * parse args - NOTICE: atoi() cannot throw errors - assumption made that user knows correct data entries
     */
    N = atoi(argv[1]);//number of buffers
    P = atoi(argv[2]);//number of producers
    C = atoi(argv[3]);//number of consumers
    X = atoi(argv[4]);//number of items each producer thread will create
    Ptime = atoi(argv[5]);//(seconds) sleep time for producers after item creation
    Ctime = atoi(argv[6]);//(seconds) sleep time for consumers after item consumption
    arraySize = P*X; //Number of Producers * Number of items each will create
    number_to_Consume = (P*X)/C;
    int leftover = (P*X) % C;
    bool LEFTOVER_FLAG = 0;
    if(leftover > 0) {
        LEFTOVER_FLAG = 1;
    }

    printf("Number of Buffers: %d\n"
           "Number of Producers: %d\n"
           "Number of Consumers: %d\n"
           "Number of Items per Producer: %d\n"
           "Number of Items each Consumer will consume: %d\n"
           "Sleep time (seconds) for Producers: %d\n"
           "Sleep time (seconds) for Consumers: %d\n",N,P,C,X,number_to_Consume,Ptime,Ctime);

    buffer = calloc(N,sizeof(int)); //initializes buffer
    producer_Array = calloc(arraySize,sizeof(int));
    consumer_Array = calloc(arraySize,sizeof(int));
    printf("Size of test arrays: %d\n",arraySize);

    clock_gettime(CLOCK_REALTIME, &start_time);

    /**
        spawn all threads
    */
    int total_Threads = P + C;
    int scraps = 1;
    struct thread_info tinfo[total_Threads];
    if(LEFTOVER_FLAG){
        printf("Over-consume is: ON\n"
               "Consumer Thread: 1 will consume %d items.\n\n",number_to_Consume+leftover);
    }else{
        printf("Over-consume is: OFF\n\n");
    }
    int i,k;
    int j = 0;
    for(i = 0; i < total_Threads; i++) //create Threads
    {
        if(i < P) {
            tinfo[i].readable_id = i + 1;
            tinfo[i].type = "Producer";
            pthread_create(&tinfo[i].tid, NULL, producer_Thread, (void *) &tinfo[i]);
        }else{
            printf("***CREATED CONSUMER***\n");
            tinfo[i].readable_id = j + 1;
            tinfo[i].type = "Consumer";
            tinfo[i].consumer_num = number_to_Consume;
//            if(LEFTOVER_FLAG && i == P){
//                tinfo[i].consumer_num = number_to_Consume + leftover;
//            }
            if(LEFTOVER_FLAG && i >= P){
                if(scraps <= leftover) {
                    tinfo[i].consumer_num = number_to_Consume + 1;
                    scraps++;
                }
            }
            pthread_create(&tinfo[i].tid, NULL, consumer_Thread, (void *) &tinfo[i]);
            j++;
        }
    }

//    if(LEFTOVER_FLAG && i == total_Threads-1){
//        tinfo[total_Threads-1].consumer_num = number_to_Consume + leftover;
//    }

    /**
        join all threads
    */
    for( k = 0; k < total_Threads; k++)
    {
        pthread_join(tinfo[k].tid,NULL);
        printf("%s Thread joined: %d\n",tinfo[k].type,tinfo[k].readable_id);
    }

    clock_gettime(CLOCK_REALTIME, &end_time);

    time_t now;
    time(&now);
    printf("Current time: %s\n",ctime(&now));

    //run test strat

    seconds = end_time.tv_sec - start_time.tv_sec;
    nano_seconds = end_time.tv_nsec - start_time.tv_nsec;
    if(end_time.tv_nsec < start_time.tv_nsec){
        seconds--;
        nano_seconds = nano_seconds + 1000000000L;
    }

    insertionSort(consumer_Array,arraySize);

    printf("Producer Array\t|  Consumer Array\n");
    for(int z = 0; z < arraySize; z++){
        //producer_Array[i] = i+1;
        //consumer_Array[i] = i+1;

        printf("%d\t\t|  %d\n",producer_Array[z],consumer_Array[z]);
        if(producer_Array[z] != consumer_Array[z]){
            arrays_Matching = false; //array values do not match
        }
    }

    if(arrays_Matching){
        printf("\nProducer and Consumer arrays match!\n");
    }else{
        printf("\nUh oh! Something went wrong and the arrays do not match!\n");
    }
    printf("\nProducer Sum: %d\nConsumer Sum: %d\n",producer_sum,consumer_sum);


    printf("\nTotal runtime: %ld.%09ld seconds\n",seconds,nano_seconds);
    return 0;
}
