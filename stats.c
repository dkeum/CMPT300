#include "stats.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

typedef struct stats { //define struct
	int factorynumber;
	int made;
	int eaten;
	double min_delay_ms;
	double avg_delay_ms;
	double max_delay_ms;
	double total_delay_ms;
} stats_t;

stats_t *factorystats; //struct pointer variable
sem_t stats_mutex;
int numberfactory = 0;

void stats_init(int num_producers) {
	
	factorystats = (stats_t*)malloc(num_producers*sizeof(stats_t));
	for (int i = 0; i < num_producers; i++) { //assign values in structure using for loop
		factorystats[i].factorynumber = i;
		factorystats[i].made = 0;
		factorystats[i].eaten = 0;
		factorystats[i].min_delay_ms = -1;
		factorystats[i].avg_delay_ms = -1;
		factorystats[i].max_delay_ms = -1;
		factorystats[i].total_delay_ms = 0;
	}
	
	sem_init(&stats_mutex, 0, 1); // other processes cannot share
	numberfactory = num_producers;
}

void stats_cleanup(void) {	
	free(factorystats);
	factorystats = NULL;
}

void stats_record_produced(int factory_number) {
	sem_wait(&stats_mutex); //lock
		factorystats[factory_number].made += 1; //increments made candies
	sem_post(&stats_mutex); //unlock
}

void stats_record_consumed(int factory_number, double delay_in_ms) {
	sem_wait(&stats_mutex); //controlls access to stats
	
	factorystats[factory_number].eaten += 1; //increments eaten candies
	if (factorystats[factory_number].min_delay_ms == -1) {
		factorystats[factory_number].max_delay_ms = delay_in_ms;
		factorystats[factory_number].total_delay_ms = delay_in_ms;
		factorystats[factory_number].min_delay_ms = delay_in_ms;
		factorystats[factory_number].total_delay_ms = delay_in_ms;
	}
	
	else {
		if (factorystats[factory_number].min_delay_ms > delay_in_ms) {
			factorystats[factory_number].min_delay_ms = delay_in_ms;
		}
		else if (factorystats[factory_number].max_delay_ms < delay_in_ms) {
			factorystats[factory_number].max_delay_ms = delay_in_ms;
		}
		factorystats[factory_number].total_delay_ms += delay_in_ms; //total delay gets incremented by delay_in_ms
		factorystats[factory_number].avg_delay_ms = factorystats[factory_number].total_delay_ms/(double)factorystats[factory_number].eaten; //generates average delays
		
	}
	
	// if there is only one candy made, average value gets assigned the value of minimum delay
	if (factorystats[factory_number].min_delay_ms == factorystats[factory_number].max_delay_ms) {
		factorystats[factory_number].avg_delay_ms = factorystats[factory_number].min_delay_ms;
	}

	sem_post(&stats_mutex); //unlock
}

void stats_display(void) {
	printf("Statstics: \n");
	printf("%s%9s%10s%16s%16s%16s\n", "Factory #", "#Made", "#Eaten", "Min Delay[ms]", "Avg Delay[ms]", "Max Delay[ms]");
	
	// generates table
	for (int i = 0; i < numberfactory; i++) {
		printf("%3d%10d%9d%16.5f%16.5f%16.5f\n",
			factorystats[i].factorynumber,
			factorystats[i].made,
			factorystats[i].eaten,
			factorystats[i].min_delay_ms,
			factorystats[i].avg_delay_ms,
			factorystats[i].max_delay_ms);
		if (factorystats[i].made != factorystats[i].eaten) { // will print error if made and eaten dont match
			printf("ERROR: Mismatch between number made and eaten.\n");
		}
	}
}

