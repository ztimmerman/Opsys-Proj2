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
	int passengers;
	int weight;
	bool goingUp;
} elevator;

//useful components of passenger info
struct passenger_info{
	int desination;
	passenger_type type;
};

/*****************SYSCALLS************************/
int start_elevator(void){

	elevator.state=IDLE;
	elevator.currentFloor=BOTTOM_FLOOR;
	elevator.passengers=0;
	elevator.weight=0;
	
	return 0;

}

int issue_request(int passenger_type, int start_floor,
		  int destination_floor){

	return 0;
}

int stop_elevator(void){

	return 0;
}



/***********MODULE INITIALIZED************************/
static int elevator_init(void){
	printk(KERN_ALERT"Elevator module initialized\n");
	return 0;
}
static void elevator_exit(void){
	printk(KERN_ALERT"Elevator module de-initialized.\n");
}
module_init(elevator_init);
module_exit(elevator_exit);
