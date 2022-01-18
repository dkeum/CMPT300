#include <stdbool.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>

#include "bbuff.h"
#include <time.h>
#include <stdlib.h>

#define BUFFER_SIZE 10

//variables
void* buffer[BUFFER_SIZE];
int avail_slots_buffer;
sem_t full_buffer;
sem_t empty_buffer;
sem_t mutex; //used for acquiring and releasing lock

//initialize bounded_buffer
void bbuff_init(void)
{	
	sem_init(&full_buffer, 0, 0);
	sem_init(&empty_buffer, 0, BUFFER_SIZE);
	sem_init(&mutex, 0, 1);
	avail_slots_buffer = 10;
	return;
}

void bbuff_blocking_insert(void* item)
{

  //acquire lock
  sem_wait(&empty_buffer);
  sem_wait(&mutex);
  //critical section
  buffer[BUFFER_SIZE - avail_slots_buffer] = item;
  avail_slots_buffer = avail_slots_buffer -1;
  //release lock
  sem_post(&mutex);
  sem_post(&full_buffer);
  return;
}

void* bbuff_blocking_extract(void)
{
  //acquire lock
  sem_wait(&full_buffer); 
  sem_wait(&mutex); 
  
  //critical section
  void * candy_item = buffer[BUFFER_SIZE - (avail_slots_buffer+1)];//get rid of the latest candy in bbuff
  buffer[BUFFER_SIZE - avail_slots_buffer+1] = NULL;
  avail_slots_buffer = avail_slots_buffer+1; 
  //release lock
  sem_post(&empty_buffer);
  sem_post(&mutex);
  return candy_item;
}


//buffer is empty
_Bool bbuff_is_empty(void)
{
  //if this is true, no candy is in buffer; 
  if(avail_slots_buffer == BUFFER_SIZE){
    return true; 
  }
  else{
    return false;
  }
  
}




