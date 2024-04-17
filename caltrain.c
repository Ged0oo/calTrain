#include "caltrain.h"


/* 
 * Initializes the station struct and associated mutexes and condition variables.
 */
void station_init(struct station *station)
{
	station->wait_passengers = 0;
	station->on_board = 0;

	// Mutexes are used to provide exclusive access to shared resources. 
	pthread_mutex_init(&(station->train_mutex) , NULL);

	// initializes the condition variables used to coordinate threads waiting for a certain condition to become true. 
	pthread_cond_init(&(station->condition_arrival) , NULL);
	pthread_cond_init(&(station->condition_full) , NULL);
}


/*
 * Simulates the arrival of a train with a given number
 * of empty seats. It broadcasts the arrival signal to waiting
 * passengers and waits until all passengers have boarded the train.
 */
void station_load_train(struct station *station, int count)
{
	/* sets the number of available seats on the arriving train. */ 
	station->empty_places = count;

	/*
	 * send signal for all threads waiting on the condition_arrival
	 * Allowing waiting passengers to wake up and attempt to board the train.
	 */
	pthread_cond_broadcast(&(station->condition_arrival));

	/*
	 * The thread locks the train_mutex to ensure mutual exclusion 
	 * and access to shared data in the station structure.
	 */
	pthread_mutex_lock(&(station->train_mutex));	

	/*
	 * The loop is used to wait until the condition for the train to depart is satisfied.
	 * This condition checks if there are passengers currently on board the train. 
	 * Then the condition checks if there are passengers waiting at the station and there are empty seats available on the train
	 */
	while(station->on_board != 0 || ((station->wait_passengers != 0) && (station->empty_places != 0)))
	{
		/* 
		 * Release the train_mutex lock to make other threads satisfy the condition_full condition
		 * By releasing the mutex, it allows other threads to acquire the mutex and potentially 
		 * update station variables while waiting for the train to depart. 
		*/
		pthread_cond_wait(&(station->condition_full) ,&(station->train_mutex));
	}
	
	/* The thread releases the train_mutex */
	pthread_mutex_unlock(&(station->train_mutex));
	
	station->empty_places = 0;
}


/*
 * Simulates a passenger waiting for a train to arrive.
 * It increments the number of waiting passengers and waits 
 * until there are empty seats available on the train.
 */
void station_wait_for_train(struct station *station)
{
	/*
	 * The thread locks the train_mutex to ensure mutual exclusion 
	 * and access to shared data in the station structure.
	 */
	pthread_mutex_lock(&(station->train_mutex));
	

	/* increment number of passengers */
	station->wait_passengers++;
	

	/*
	 * This loop waits until there are empty places available on the train
	 */
	while(station->empty_places == 0)
	{
		/*
		 * This line waits on the condition_arrival condition variable. 
		 * When the train arrives and signals on this condition variable, 
		 * the thread will wake up and continue execution.
		 */
		pthread_cond_wait(&(station->condition_arrival) , &(station->train_mutex));
	}

	
	/* After waking up, 
	 * the thread decrements the passengers to indicate that one passenger has boarded the train. 
	 * It also decrements empty_places to reflect that one seat is occupied.
	 * increments the number of passengers currently on board the train.
	*/
	station->wait_passengers--;
	station->empty_places--;
	station->on_board++;

	/* The thread releases the train_mutex */
	pthread_mutex_unlock(&(station->train_mutex));
}

/*
 * Simulates a passenger boarding the train. 
 * It decrements the number of passengers on board 
 * and signals the train when all passengers have boarded.
 */
void station_on_board(struct station *station)
{
	/*
	 * The thread locks the train_mutex to ensure mutual exclusion 
	 * and access to shared data in the station structure.
	 */
	pthread_mutex_lock(&(station->train_mutex));
	
	/*
	 * This line decrements the number of passengers currently on board, 
	 * indicating that a passenger has been boarded.
	 */
	station->on_board--;
	
	/*
	 * This condition checks if there are no more passengers on board. 
	 * It means that the train is full or has departed. In that case,
	 */
	if(station->on_board == 0)
	{
		/*
		 * The function signals the condition_full condition variable 
		 * to notify other waiting threads that the train is full or has departed.
		 */
		pthread_cond_signal(&(station->condition_full));
	}
	
	/* The thread releases the train_mutex */
	pthread_mutex_unlock(&(station->train_mutex));
}
