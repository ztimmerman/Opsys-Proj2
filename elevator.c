//File for the elevator kernel module


#include <linux/init.h>		//initiating module
#include <linux/module.h>	//creating module
#include <linux/mutex.h>	//mutex locks
#include <linux/kthread.h>	//for creating thread
#include <linux/slab.h>		//kmalloc
#include <linux/list.h>		//for linked list

MODULE_LICENSE("Dual BSD/GPL");

/*********************CONSTANTS*******************************/
//defining the constants given in the project description
#define MAX_WEIGHT 15
#define MAX_PASSENGER 10
#define TOP_FLOOR 10
#define BOTTOM_FLOOR 1


/************************ENUMS********************************/
//defining an enum for the 5 states of the elevator
typedef enum{
	OFFLINE=0,
	IDLE=1,
	LOADING=2,
	UP=3,
	DOWN=4,
} elevator_state;

//defining the 4 types of passenger
typedef enum{
	ADULT=1,
	CHILD=2,
	SERVICE=3,
	BELLHOP=4,
} passenger_type;

/******************STRUCTS*************************/
//useful components of the elevator's information
struct elevator_info{
	elevator_state state;
	int currentFloor;
	int destination;
	int space;
	int weight;
	bool deactivating;	//is the module being removed
	bool goingUp;
	struct list_head queue;	//linked list
} elevator;

//useful components of passenger info
struct passenger_info{
	int destination;
	int currentFloor;
	passenger_type type;
	bool goingUp;	//which direction are they going
	struct list_head passengerList;	//individual nodes for linked-list
} passenger;

//components of floor info
struct floor_info{
	int served;
	int waiting;
	struct list_head queue;	//linked list
};

/******************************GLOBALS***************************************/
struct floor_info floors[TOP_FLOOR];	//array of floors
struct task_struct *elevatorThread;	//thread for elevator
struct mutex elevatorLock = __MUTEX_INITIALIZER(elevatorLock);
//for locking elevator
struct mutex floorLock = __MUTEX_INITIALIZER(floorLock);
//for locking floors

/******************************MAIN FUNCTION DECLARATION********************/
int elevator_main(void * data);

/*****************************************SYSCALLS*****************************/
/******START ELEVATOR*************/
extern int(* STUB_start_elevator)(void);

int start_elevator(void){

	mutex_lock(&elevatorLock);	
	//lock elevator

	if(elevator.state==OFFLINE){		//if the elevator is OFFLINE

		//set default properties
		printk(KERN_ALERT"Starting elevator\n");
		elevator.state=IDLE;
		elevator.currentFloor=BOTTOM_FLOOR;
		elevator.space=0;
		elevator.weight=0;
		elevator.deactivating=0;

		//create a thread for the main elevator function
		//if successfull thread will run elevator_main()
		elevatorThread=kthread_run(elevator_main,NULL,"elevator_main");

		//if thread failed to create
		if(IS_ERR(elevatorThread) !=0){
			printk(KERN_ALERT"ERROR. Elevator thread was not created.");
			mutex_unlock(&elevatorLock);
			return -ENOMEM;
		}
		else{	//thread created succesfully
			mutex_unlock(&elevatorLock);
			return 0;
		}
	}
	
	//elevator was not OFFLINE
	//unlock levator
	mutex_unlock(&elevatorLock);
	return 1;
}


/******ISSUE REQUEST*************/
extern int(* STUB_issue_request)(int pass_type, int start_floor, int desired_floor);

int issue_request(int pass_type, int start_floor, int desired_floor){

	//invalid arguments
	if((pass_type>4||pass_type<1)||(start_floor<BOTTOM_FLOOR||start_floor>TOP_FLOOR)||
	  (desired_floor<BOTTOM_FLOOR||desired_floor>TOP_FLOOR)){

		printk(KERN_ALERT"Request Invalid. Passneger type or Floor out of range.");
		return 1;
	}
	else{	//arguments valid for a new passenger
		//storage for arguments
		struct passenger_info *passenger=NULL; 

		//allocate memory for new passenger
		//GFP_KERNEL flag ensures memory
		passenger=kmalloc(sizeof(struct passenger_info),GFP_KERNEL);

		//set new passenger information
		passenger->type=pass_type;
		passenger->destination=desired_floor;
		passenger->currentFloor=start_floor;
		if(passenger->destination>passenger->currentFloor)
			passenger->goingUp=1;
		else
			passenger->goingUp=0;

		//create new passenger node
		INIT_LIST_HEAD(&passenger->passengerList);

		mutex_lock(&elevatorLock);	//lock elevator
		
		//adding new passenger to the queue on correct floor
		list_add_tail(&passenger->passengerList,&floors[passenger->currentFloor-1].queue);
		//display new passenger on waiting count
		floors[passenger->currentFloor-1].waiting++;
	
		mutex_unlock(&elevatorLock);
		
		printk(KERN_ALERT"New passenger request made.\n");

		return 0;
	}
}
/******STOP ELEVATOR*************/
extern int(* STUB_stop_elevator)(void);

int stop_elevator(void){

	mutex_lock(&elevatorLock);	//lock elevator

	printk(KERN_ALERT"Stopping elevator\n");
	
	//elevator is deactivating
	if(elevator.deactivating==1){
		mutex_unlock(&elevatorLock);
		return 1;
	}

	//set elevator to deactivate
	elevator.deactivating=1;
	mutex_unlock(&elevatorLock);

	return 0;
}

/********************************ELEVATOR_MAIN********************************/
//IMPLEMENT ELEV SEARCHING ALGORITHM, PASS LOAD/UNLOAD
int elevator_main(void * data){

	//while IDLE, scan floors for waiting passengers
	//When found
	//Move to floor, LOADING passengers going UP or DOWN
	//Move to furthest destination floor in UP or DOWN direection
	//Check each floor, pick up passengers desiring same direction

	//If space==0 and all floors empty IDLE
	//if deactivating==1, unload passengers, do not pick up more
	//Set OFFLINE when unloaded

	elevator.state=OFFLINE;
	kthread_stop(elevatorThread);

	return 0;
}

/************************************PROC*************************************/
//SET UP PROC FILE WITH FORMAT GIVEN IN DESCRIPTION


/**************************************MODULE*********************************/
/***********MODULE INITIALIZED************************/
static int elevator_init(void){			//initializing elevator'

	int i;

	mutex_init(&elevatorLock);
	mutex_init(&floorLock);

	mutex_lock(&elevatorLock);	//set lock to init elevator properties

	elevator.state=OFFLINE;
	elevator.currentFloor=BOTTOM_FLOOR;
	elevator.space=0;
	elevator.weight=0;
	elevator.deactivating=0;
	INIT_LIST_HEAD(&elevator.queue);	//link-list of passengers on elevator

	mutex_unlock(&elevatorLock);

	mutex_lock(&floorLock);		//set lock to init elevator properties
	for(i=0;i<TOP_FLOOR;i++){
		floors[i].waiting=0;
		floors[i].served=0;
		INIT_LIST_HEAD(&floors[i].queue);//link-list of passenger queue
	}
	mutex_unlock(&floorLock);

	STUB_start_elevator=start_elevator;
	STUB_issue_request=issue_request;
	STUB_stop_elevator=stop_elevator;

	printk(KERN_ALERT"Elevator module initialized\n");
	return 0;
}
static void elevator_exit(void){

	STUB_start_elevator=NULL;
	STUB_issue_request=NULL;
	STUB_stop_elevator=NULL;
	
	printk(KERN_ALERT"Elevator module being removed.\n");

	mutex_lock(&elevatorLock);

	if(elevator.state!=OFFLINE){		//start shutting down elevator
		elevator.deactivating=1;
		//kthread_stop(elevatorThread);
		//sets deactivate to  1, then should wait until
		//elevator_main completes (unsure)
	}
	
	mutex_unlock(&elevatorLock);

	mutex_destroy(&elevatorLock);
	mutex_destroy(&floorLock);
}
module_init(elevator_init);
module_exit(elevator_exit);
