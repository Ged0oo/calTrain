#include <pthread.h>

struct station 
{
	pthread_mutex_t train_mutex;
	pthread_cond_t condition_full;
	pthread_cond_t condition_arrival;

	int on_board;
	int empty_places;
	int wait_passengers;
};

void station_init(struct station *station);

void station_load_train(struct station *station, int count);

void station_wait_for_train(struct station *station);

void station_on_board(struct station *station);