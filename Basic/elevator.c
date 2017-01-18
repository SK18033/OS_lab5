// Header files used for the Basic Elevator Program
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>


// Data stucture to hold the person properties
struct person {
	int id;
	int from_floor, to_floor; 
	double arrival_time;
};

//Data stucture for Doubly linked list np
struct dlnode {
	struct person* pdata;
	struct dlnode* prev;
	struct dlnode* next;
};

//Maintain the Ends of Doubly linked list
struct dll {
	int size ;
	struct dlnode *head ;
	struct dlnode *tail ;
};


//Data structure for the elevator properties
struct elevator {
	int id;
	int current_floor;
	struct dlnode *nnode;
};


// Data structure for all the varables used globally by the threads	
struct gv { 
	int num_elevators;		// number of elevatr threads
	int num_floors;			// Maximum number of floors
	int arrival_time;		// Time interval to generate persons
	int elevator_speed;		// Speed of the elevator to move
	int simulation_time;		// Total time for the program to run
	int random_seed;		// To generate random floors for the persons
	int num_people_started; 	// Keep track of number of persons arrived
	int num_people_finished;	// keep track of number of persons utilised the elevator
	pthread_mutex_t *lock; 		// Lock for accessing the doubly linked list
	pthread_mutex_t *elock;		// elevator Lock for accessing the doubly linked list
	pthread_cond_t *ecv;		// elevator conditional wait on the doubly linked list
	double start_time;
	double end_time;
	int is_runing;
};


// Global declerations
struct dll *dlist;	
struct gv *gvar;


//Person Generator Method
void* person_generator()
{
	int i = 0; 
	time_t sec;
	double cur_time = 0.0 , start_time = 0.0;

	sec = time(NULL);
	start_time = (double)sec;			// Start time for the simulation

	srand(gvar->random_seed);			// To alter the seed for every thread generated

	while(1)
	{
		sec = time(NULL);
		cur_time = (double)sec;		// Time taken for a person to create
		if(cur_time - gvar->start_time >= (double)gvar->simulation_time)
		{
			gvar->end_time = cur_time;
			pthread_mutex_lock(gvar->elock);
			pthread_cond_broadcast(gvar->ecv);
			pthread_mutex_unlock(gvar->elock);
			gvar-> is_runing = 0;
			return NULL;
		}
		
		
		struct dlnode* node;
		struct person* psn;
		
		// Dynamic memory Allocation for the above created variables
		node = malloc(sizeof(struct dlnode));
		psn = malloc(sizeof(struct person));

		// Initialization of the members		
		node->pdata = NULL;
		node->prev = NULL;
		node->next = NULL;

		// Assignment of the member values with random floors between 0 to Maximum limit
		psn->id = i;
		i++;
		do{
			psn->from_floor = rand()% gvar->num_floors;
			psn->to_floor = rand()% gvar->num_floors;		
		}while(psn->from_floor == psn->to_floor);	// To avoid same from_floor and to_floor
//			break;
		
		sec = time(NULL);
		
		node->pdata = psn;		

		pthread_mutex_lock(gvar->lock);		// Lock is enabled in order to have update the doubly linked list

		
		// For the first insertion of the data
		if(dlist->tail == NULL )
		{
			(dlist->size)++;		// Maintaining the number of entries made
			dlist->tail = node;
			dlist->head = dlist->tail;
		}	
		else
		{
			(dlist->size)++;
			dlist->tail->next = node;
			node->prev = dlist->tail;
			dlist->tail = node;

			
		}

		pthread_mutex_unlock(gvar->lock);	// Lock is Disabled in order to have the elevator access the list
		pthread_mutex_lock(gvar->elock);	// Elevator lock for doubly linked list to be unlocked by person thread		
		pthread_cond_signal(gvar->ecv);		//Wakeup Signal
		pthread_mutex_unlock(gvar->elock);	// Elevator lock is diabled


		psn->arrival_time = (double)sec;	// Record the time person arrived at certain floor and requested for Elevator
		
		
		printf("[%0.3lf] Person %d arrives on floor %d, waiting to go for floor %d\n", psn->arrival_time-gvar->start_time, psn->id, psn->from_floor, psn->to_floor);
		//printf("Creating person %d\n ", i-1);

		sleep(gvar->arrival_time);		// Generate person after sleeping for p seconds 
	} 
}


void* elevator(void *etids)
{

	int P=0,Q=0;

	time_t sec;
	double cur_time = 0.0 , start_time = 0.0;
	
	struct elevator *elev;
	elev = malloc(sizeof(struct elevator));
	

	elev->id = *((int *)etids);

	// Dynamic memory Allocation for the above created elevator 
	elev->current_floor = 0;

	
	
	sec = time(NULL);
//	start_time = (double)sec;					// Record the time when elevator starts 
	
//	printf("====[%0.3lf] Elevator %d created\n", (double)sec-gvar->start_time, elev->id);


	while(1)
	{
		sec = time(NULL);
		cur_time = (double)sec;	
		if(cur_time - gvar->start_time >= (double)gvar->simulation_time)
		{
			gvar->end_time = cur_time;
			gvar-> is_runing = 0;
			return NULL;
		}		
		
		struct person *psn;
		psn = (struct person*)malloc(sizeof (struct person));
		
		while(dlist->tail == NULL && gvar-> is_runing)
		{
			pthread_mutex_lock(gvar->elock);			// Elevator lock for doubly linked list to be unlocked by person thread		
			//printf("Elevator Waits!\n");
			pthread_cond_wait(gvar->ecv, gvar->elock);	// Waits for the linked lists to be free

			pthread_mutex_unlock(gvar->elock);			// Elevator lock is diabled

		}
			sec = time(NULL);
//		printf("====[%0.3lf] Elevator %d created\n", (double)sec-gvar->start_time, elev->id);		
		if(!gvar-> is_runing)
			return NULL;	
//		printf("Elevator Wakes up! %d \n" ,*((int *)etids));

		
		
		pthread_mutex_lock(gvar->lock);				// Lock is enabled in order to have update the doubly linked list			

		
		psn->id = dlist->head->pdata->id;
		psn->from_floor = dlist->head->pdata->from_floor;
		psn->to_floor = dlist->head->pdata->to_floor;


		// Deletion of the head node => Person is dropped 
		struct dlnode* temp = dlist->head;		
	
		dlist->head = dlist->head->next;

		if(dlist->head != NULL)	
			dlist->head->prev = NULL;
		else
			dlist->tail = NULL;

		gvar->num_people_started++;


		// Free the memory of the tail node
		free(temp);
		pthread_mutex_unlock(gvar->lock);			// Lock is Disabled in order to have the other elevators or person thread access the list

		
	 	// Assignment of the variables for elevator and its motion
		
		int change;
		if(elev->current_floor != psn->from_floor)
		{
			sec = time(NULL);
			printf("[%0.3lf] Elevator %d starts moving from %d to %d\n", (double)sec-gvar->start_time, elev->id,elev->current_floor,psn->from_floor);
			if((psn->from_floor - elev->current_floor) > 0)
				change = 1;
			else
				change = -1;
			while(elev->current_floor != psn->from_floor)
			{
				elev->current_floor += change;
				sleep(gvar->elevator_speed);
				cur_time = (double)time(NULL);	
				if(cur_time - gvar->start_time >= (double)gvar->simulation_time)
				{
					gvar->end_time = cur_time;
					gvar-> is_runing = 0;
					return NULL;
				}
			}
		}
		elev->current_floor = psn->from_floor;
		sec = time(NULL);
		printf("[%0.3lf] Elevator %d picks up Person %d\n", (double)sec-gvar->start_time, elev->id,psn->id);
		printf("[%0.3lf] Elevator %d starts moving from %d to %d\n", (double)sec-gvar->start_time, elev->id,elev->current_floor,psn->to_floor);
		
		if((psn->to_floor - elev->current_floor) > 0)
			change = 1;
		else
			change = -1;


		while(elev->current_floor != psn->to_floor)
		{

			elev->current_floor += change;
			sleep(gvar->elevator_speed);
			cur_time = (double)time(NULL);	
			if(cur_time - gvar->start_time >= (double)gvar->simulation_time)
			{

				gvar-> end_time = cur_time;
				gvar-> is_runing = 0;
				return NULL;

			}

		}

		pthread_mutex_lock(gvar->lock);				// Lock is enabled in order to have update the doubly linked list			
		gvar->num_people_finished++;
		pthread_mutex_unlock(gvar->lock);			// Lock is Disabled in order to have the other elevators or person thread access the list


		sec = time(NULL);
		printf("[%0.3lf] Elevator %d arrives at floor %d\n", (double)sec-gvar->start_time, elev->id,elev->current_floor);
		printf("[%0.3lf] Elevator %d drops Person %d\n", (double)sec-gvar->start_time, elev->id,psn->id);
//		printf("Here/.....");fflush(stdout);

		cur_time = (double)sec;

	}

}


int main(int argc, char* argv[])
{
	int		i=0, j=0, No_of_Persons = 1;
	int             *ptids, *etids;  
  	pthread_t       *pthrds, *ethrds;
	pthread_attr_t  *pattrs, *eattrs;
	void            *pretval, *eretval;
	
	// Arguements validation
	if(argc != 7)
	{
		perror("Invalid Number of arguemnts\n");
		exit(1);
	}


	// Allocating Memory for the Global Variables
	gvar = malloc(sizeof(struct gv));
	
	
	gvar->num_elevators		= atoi(argv[1]);
	gvar->num_floors	 	= atoi(argv[2]);
	gvar->arrival_time 		= atoi(argv[3]);
	gvar->elevator_speed 		= atoi(argv[4]);
	gvar->simulation_time 		= atoi(argv[5]);
	gvar->random_seed 		= atoi(argv[6]);



	// Validating Inputs
	if(gvar->num_elevators < 0)
	    {
	      fprintf(stderr, "arg 2 : Atleast 1 elevator needs to be created \n");
	      exit(1);
	    }

	if(gvar->num_floors < 0)
	    {
	      fprintf(stderr, "arg 3 : Atleast 1 floor is needed for the elevator to toggle \n");
	      exit(1);
	    }
	if(gvar->arrival_time < 0)
	    {
	      fprintf(stderr, "arg 4 : Arrival time cannot be 0secs \n");
	      exit(1);
	    }
	if(gvar->elevator_speed < 0)
	    {
	      fprintf(stderr, "arg 5 : Elevator speed cannot be 0 floors/sec\n");
	      exit(1);
	    }
	if(gvar->simulation_time < 0)
	    {
	      fprintf(stderr, "arg 6 : The total elevator simulation time cannot be zero \n");
	      exit(1);
	    }

	gvar->num_people_started 	= 0;
	gvar->num_people_finished 	= 0;
	gvar-> is_runing		= 1;



	// Allocating Memory for the Global Variables
	gvar->lock 			= (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	gvar->elock 			= (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	gvar->ecv 			= (pthread_cond_t *)malloc(sizeof(pthread_cond_t)); 
	
	
	pthread_mutex_init(gvar->lock,NULL);
	pthread_mutex_init(gvar->elock,NULL);
	pthread_cond_init(gvar->ecv,NULL);

		
	if(gvar->num_elevators <=0 || gvar->num_floors <=0 || gvar->arrival_time <=0 ||
	   gvar->elevator_speed <=0 || gvar->simulation_time <=0 || gvar->random_seed <=0 )
	{
		perror("Input needs to be greater than zero\n");
		exit(1);
	}
	
	time_t sec = time(NULL);
	gvar->start_time = (double)sec;
	dlist = malloc(sizeof(struct dll));

	dlist->size = 0;
	dlist->head = NULL;
	dlist->tail = NULL;
	
	ethrds 			= (pthread_t*) 	    malloc(sizeof (pthread_t)* gvar->num_elevators);
	eattrs			= (pthread_attr_t*) malloc(sizeof (pthread_attr_t)* gvar->num_elevators);
	etids  			= (int*) 	    malloc(sizeof (int)* gvar->num_elevators);

	pthrds 			= (pthread_t*) 	    malloc(sizeof (pthread_t)* No_of_Persons);
	pattrs			= (pthread_attr_t*) malloc(sizeof (pthread_attr_t)* No_of_Persons);
	ptids  			= (int*) 	    malloc(sizeof (int)* No_of_Persons);

	// Create Elevator threads
	for(i = 0; i < gvar->num_elevators; i++) 
	{
		if(pthread_attr_init(eattrs+i)) 
			perror("Elevator attr_init()");
		
		etids[i] = i;
		
		if(pthread_create(ethrds+i, eattrs+i, elevator, etids+i) != 0) 
			perror("Elevator pthread_create()");

		//printf("Creating Elevator %d\n", i);

	}


	// Create Elevator threads
	for(j = 0; j < No_of_Persons; j++) 
	{
		if(pthread_attr_init(pattrs+j)) 
			perror("Person attr_init()");
			
		ptids[j] = j;
		
		if(pthread_create(pthrds+j, pattrs+j, person_generator, ptids+j) != 0) 
			perror("Person pthread_create()");
		
		//printf("Creating Person %d\n", j);
	}

	sleep(gvar->simulation_time);

	// join threads
	for(i = 0; i < gvar->num_elevators; i++)
		pthread_join(ethrds[i], &eretval);

	for(j = 0; j < No_of_Persons; j++)
		pthread_join(pthrds[i], &pretval);
	
	printf("Simulation result: %d people have started, %d people have finished during %lf seconds.\n",gvar->num_people_started,gvar->num_people_finished,gvar->end_time-gvar->start_time);


	free(eattrs);
	free(ethrds);
	free(etids);
	
	free(pattrs);
	free(pthrds);
	free(ptids);

	return 0;
	
}
