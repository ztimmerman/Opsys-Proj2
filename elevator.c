//File for the elevator kernel module


#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");

//defining the constants given in the project description
#define MAX_WEIGHT 15
#define MAX_PASSENGER 10
#define TOP_FLOOR 10
#define BOTTOM_FLOOR 1


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
	ADULT=0,
	CHILD=1,
	SERVICE=2,
	BELLHOP=3,
} passenger_type;

//useful components of the elevator's information
struct elevator_info{
	elevator_state state;
	int currentFloor;
	int destination;
	int passengers;
	int weight;
	bool deactivating;
	bool goingUp;
} elevator;

//useful components of passenger info
struct passenger_info{
	int desination;
	int currentFloor;
	passenger_type type;
	bool goingUp;
} passenger;

//components of floor info
struct floor_info{
	int served;
	int waiting;
};

/*****************SYSCALLS************************/
/******START ELEVATOR*************/
extern int(* STUB_start_elevator)(void);

int start_elevator(void){

	printk(KERN_ALERT"Start elevator\n");
	return 0;
}


/******ISSUE REQUEST*************/
extern int(* STUB_issue_request)(int pass_type, int start_floor, int desired_floor);

int issue_request(int pass_type, int start_floor, int desired_floor){
	printk(KERN_ALERT"Issue request\n");

	return 0;
}
/******STOP ELEVATOR*************/
extern int(* STUB_stop_elevator)(void);

int stop_elevator(void){

	printk(KERN_ALERT"Stop elevator\n");
	return 0;
}

/***********MODULE INITIALIZED************************/
static int elevator_init(void){			//initializing elevator
	elevator.state=OFFLINE;
	elevator.currentFloor=BOTTOM_FLOOR;
	elevator.passengers=0;
	elevator.weight=0;

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
	
	printk(KERN_ALERT"Elevator module de-initialized.\n");
}
module_init(elevator_init);
module_exit(elevator_exit);
