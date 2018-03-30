//File for the elevator kernel module


#include <linux/init.h>		//initiating module
#include <linux/module.h>	//creating module
#include <linux/mutex.h>	//mutex locks
#include <linux/kthread.h>	//for creating thread
#include <linux/slab.h>		//kmalloc
#include <linux/list.h>		//for linked list
#include <linux/delay.h>	//msleep
#include <linux/proc_fs.h>
#include <linux/seq_file.h>	//seq_file

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
static struct file_operations fops;     //proc file

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

/**************************************MAIN FUNCTIONS*************************/
int find_size(passenger_type p) {
	if ((p == CHILD) | (p == ADULT)) { return 1; }
	return 2;
}

int find_weight(passenger_type p) {
	if (p == ADULT) {
		return 1*10;
	}
	else if (p == CHILD) {
		return 0.5*10;
	}
	else if (p == SERVICE) {
		return 2*10;
	}
	else {
		return 3*10;
	}
}

void set_state(void) {					// figure out if the elevator is going up or down and set state accordingly
	if (elevator.destination > elevator.currentFloor) { elevator.state = UP; }
	else {elevator.state = DOWN;}
}

void set_destination(void) {
	struct list_head * temp;
	struct list_head * dummy;
	struct passenger_info * pass;
	// look through everyone in the elevator. find the person who is going to the floor closest to currentFloor, and make that floor the destination
	if (elevator.space == 0) { return; }	// do nothing if the elevator is empty

	list_for_each_safe(temp, dummy, &elevator.queue) {
		// If pass->destination is closer than the elevator's destination, set elevator.destination to pass->destination.
		pass = list_entry(temp, struct passenger_info, passengerList);
		if (pass->destination > elevator.currentFloor && pass->destination - elevator.currentFloor < elevator.destination - elevator.currentFloor) {
			elevator.destination = pass->destination;
		}
		else if (elevator.currentFloor - pass->destination < elevator.currentFloor - elevator.destination) {
			elevator.destination = pass->destination;
		}
	}

	set_state();
}

void load_elevator(void) {					// loads the elevator on the current floor
	bool goingup;									// this is used for comparisons with a passenger's goingUp value. (since state will be LOADING)
	struct list_head * temp;
	struct list_head * dummy;
	struct passenger_info * pass;
	if (elevator.deactivating == 1) {return;}											// No one can enter the elevator while it is deactivating.
	if ((elevator.space == MAX_PASSENGER) | ((elevator.weight/10) == MAX_WEIGHT)) { return; }		// do nothing if the elevator is full.
	if (floors[elevator.currentFloor].waiting == 0) { return; }							// do nothing if there's no one to load.

	if (elevator.state == UP) {goingup = true;}
	else if (elevator.state == DOWN){goingup = false;}

	elevator.state = LOADING;
	// wait for 1.0 seconds *********************
	msleep(1000);


	if (elevator.space == 0) {	// if no one is in the elevator, we don't need to check the direction for the first passenger.
		pass = list_entry(&floors[elevator.currentFloor].queue, struct passenger_info, passengerList);
		list_move_tail(&floors[elevator.currentFloor].queue, &elevator.queue);		// move the first passenger on the floor into the elevator
		elevator.destination = pass->destination;
		elevator.space += find_size(pass->type);
		elevator.weight += find_weight(pass->type);
		goingup = pass->goingUp;
	}

	// load everyone on the currentFloor going in the same direction, that there is space for
	list_for_each_safe(temp, dummy, &floors[elevator.currentFloor].queue) {
		pass = list_entry(temp, struct passenger_info, passengerList);
		if (pass->goingUp == goingup) {
			if (find_size(pass->type) + elevator.space <= MAX_PASSENGER) {
				if ((find_weight(pass->type) + elevator.weight)/10 <= MAX_WEIGHT) {
					// the passenger qualifies. Move them to the elevator. Check if their destination is closer than the current one.
					list_move_tail(temp, &elevator.queue);
					if ((goingup && pass->destination < elevator.destination) | (!goingup && pass->destination > elevator.destination)) 
					{ elevator.destination = pass->destination; }	// if the new passenger's destination is closer, we're going there first.
				}
			}
		}
	}

	set_state();	
}

void unload_elevator(void) {
	struct list_head * temp;
	struct list_head * dummy;
	struct passenger_info * pass;
	if (elevator.currentFloor != elevator.destination) { return; }			// if this isn't the destination floor, do not unload.
	// wait 1.0 seconds*************************************
	msleep(1000);
	// for everyone in the elevator, if their destination is the currentFloor, get them 


	list_for_each_safe(temp, dummy, &elevator.queue) {
		// get entry for pass. If destination == currentFloor, get them out
		pass = list_entry(temp, struct passenger_info, passengerList);
		if (pass->destination == elevator.currentFloor) {
			floors[pass->currentFloor].served++;
			// remove pass's weight and size from the elevator
			elevator.space -= find_size(pass->type);
			elevator.weight -= find_weight(pass->type);

			//unload pass 
			list_del(temp);
			kfree(pass);
		}
	}


	set_destination();
}

void seek_destination(void) {							// call this when the elevator is empty and it needs to seek out new passengers
	bool found = false;
	int i;
	for (i = BOTTOM_FLOOR; i <= TOP_FLOOR; i++) {
		if (floors[i-1].waiting > 0) { elevator.destination = i; found = true; }
		if (found == false) { elevator.state = IDLE; }
		else { set_state(); }
	}
}
/********************************ELEVATOR_MAIN********************************/
//IMPLEMENT ELEV SEARCHING ALGORITHM, PASS LOAD/UNLOAD
int elevator_main(void * data) {
	while (true) {
		while (elevator.deactivating == 0) {
			//while IDLE, scan floors for waiting passengers
			while (elevator.state == IDLE) {
				load_elevator();
				
				if (elevator.state == IDLE) {		// if there were no passengers from this floor, look for people on other floors.
					seek_destination();
				}
			}
		}

		//If space==0, seek a new passengers. If none are found, set to IDLE
		if (elevator.space == 0) {
			seek_destination();
		}

		// Move one floor at a time towards elevator.destination
		// wait two seconds *************************************
		if (elevator.state == UP) { elevator.currentFloor++; msleep(2000);}
		if (elevator.state == DOWN) {elevator.currentFloor--;msleep(2000);}
		if(elevator.destination == elevator.currentFloor)unload_elevator();
		load_elevator();		//Check each floor, pick up passengers desiring same direction if deactivating == 0 and there is space



		//if deactivating==1, unload passengers, do not pick up more (handled through while loop near the top)
		//Set OFFLINE when unloaded
		if (elevator.space == 0 && elevator.deactivating == 1) {
			elevator.state = OFFLINE;
			kthread_stop(elevatorThread);
		}
	}
	return 0;
}

/************************************PROC*************************************/
//SET UP PROC FILE WITH FORMAT GIVEN IN DESCRIPTION
int elevator_show(struct seq_file *output, void *v){
	int i;
	char *status;
	
	mutex_lock(&elevatorLock);
	mutex_lock(&floorLock);
	switch(elevator.state){
		case OFFLINE:
			status="OFFLINE";
			break;
		case IDLE:
			status="IDLE";
			break;
		case UP:
			status="UP";
			break;
		case DOWN:
			status="DOWN";
			break;
		case LOADING:
			status="LOADING";
			break;

	}

	seq_printf(output, "Status:\t\t%s\n", status);
	seq_printf(output, "Current Floor:\t\t%d\n", elevator.currentFloor);
	seq_printf(output, "Next Floor:\t\t%d\n", elevator.destination);
	seq_printf(output, "Current Load:\t%d Passenger Units, %d.%d Weight Units\n",elevator.space, (elevator.weight/10),(elevator.weight%10));

	for(i=0;i<TOP_FLOOR;i++){
		seq_printf(output,"Floor %d:\t%d Waiting, %d Served\n",(i+1),floors[i].waiting,floors[i].served);

	}

	mutex_unlock(&elevatorLock);
	mutex_unlock(&floorLock);
	return 0;
}

int elevator_open(struct inode *inode, struct file *file){
	return single_open(file,elevator_show,NULL);
}


/**************************************MODULE*********************************/
/***********MODULE INITIALIZED************************/
static int elevator_init(void){			//initializing elevator'

	int i;

    	fops.open=elevator_open;	//set-up proc
        fops.read=seq_read;
        fops.release=single_release;

	mutex_init(&elevatorLock);
	mutex_init(&floorLock);

	mutex_lock(&elevatorLock);	//set lock to init elevator properties

	elevator.state=OFFLINE;
	elevator.currentFloor=BOTTOM_FLOOR;
	elevator.destination=elevator.currentFloor;
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

	proc_create("elevator",0,NULL,&fops);	//create proc

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

	remove_proc_entry("elevator",NULL);

	mutex_destroy(&elevatorLock);
	mutex_destroy(&floorLock);
}
module_init(elevator_init);
module_exit(elevator_exit);
