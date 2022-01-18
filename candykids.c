#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdint.h>
#include "bbuff.h"
#include "stats.h"


// Candy Struct
typedef struct {
  int factory_number;
  double creation_ts_ms;
} candy_t;

// Time
double current_time_in_ms(void) {
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return now.tv_sec * 1000.0 + now.tv_nsec/1000000.0;
}


_Bool stop_thread = false;


void* thread_function_factory(void* arg){
  while(!stop_thread){

    
    //pick a number of seconds which it will wait 0-3 randomly
    int waiting_time = rand()%4;
    //print msg
    printf("Factory %d ships candy & waits %ds \n", *((int*)arg) ,waiting_time);

    //dynamically allocate new candy item
      candy_t *candy = malloc(sizeof(candy_t));
      candy->factory_number = *((int*)arg);
      candy->creation_ts_ms = current_time_in_ms();
     
    //add candy item to bbuff
      bbuff_blocking_insert(candy);
      stats_record_produced(*((int*)arg));
      
    //sleep for # of sec
      sleep(waiting_time); 
  }
  printf("Candy-factory %d done\n", *((int*)arg));
  pthread_exit(NULL);
}



void* thread_function_kid(){
  while(1){
    //extract a candy item
    candy_t* candy = (candy_t*)bbuff_blocking_extract(); 
   //process item
   printf("candy is from factory %d \n",candy->factory_number);
   stats_record_consumed(candy->factory_number, current_time_in_ms() - candy->creation_ts_ms);
   int sleep_time = rand()%2;
   free(candy);
   sleep(sleep_time);
     
  }
  printf("Done! \n");
  pthread_exit(NULL);
}






int main(int argc, char *argv[]) {

  //initialized variables
   int factory = 0;
   int kids = 0;
   int seconds = 0;
    
  // 1. Extract arguments  
  if(argc <= 0){
    //display error
    printf("no args inputs");
    exit(0); 
  }
  if(argc > 0 && argc <= 4){
     factory = atoi(argv[1]);
     kids = atoi(argv[2]);
     seconds = atoi(argv[3]);
  }
	
  if (factory <= 0 || kids <= 0 || seconds <= 0){
    printf("Error in input\n");
    exit(1);
  }

  
  
  // 2. Initialize modules
  bbuff_init(); //bounded buffer 
    stats_init(factory); //initialize with factory num
  
  // 3. Launch candy-factory threads
  pthread_t factory_thread[factory]; //to hold factory threads
  int factory_numbers[factory]; //to pass factory number to thread_function_factory

  for (int i = 0; i < factory; i++) {
    factory_numbers[i] = i;
    pthread_attr_t attr;
    pthread_attr_init(&attr); // instead of attribute we can use null as well
    pthread_create(&factory_thread[i],&attr, thread_function_factory, &factory_numbers[i]);
    //error case: if threahd is not created if( thread_error == error){}
  }  
  
  // 4. Launch kid threads
  pthread_t kid_thread[kids];

  for (int i = 0; i < kids; i++) {
  pthread_attr_t attr;
  pthread_attr_init(&attr); // instead of attribute we can use null as well
  pthread_create(&kid_thread[i],&attr, thread_function_kid, NULL);
  //error case: if threahd is not created if( thread_error == error){}
 }  
  
  // 5. Wait for requested time
  for(int i=0; i<seconds; i++){
    sleep(1);
    printf("Time %ds\n", i);
  }

  
  // 6. Stop candy-factory threads

  stop_thread = true;

  for(int i=0; i<factory; i++) {
      pthread_join(factory_thread[i], NULL);
    }

  
  // 7. Wait until no more candy

  while(bbuff_is_empty() != true){
    printf("Waiting for all candy to be consumed \n");
    sleep(1);
    
  }
  // 8. Stop kid threads

  for (int i = 0; i < kids; i++) {
    pthread_cancel(kid_thread[i]);
    pthread_join(kid_thread[i],NULL);
 }  
  
  
  // 9. Print statistics

    stats_display(); 
  
  // 10. Cleanup any allocated memory

    stats_cleanup();

  return 0;
}
