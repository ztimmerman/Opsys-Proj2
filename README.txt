**************************************************
**************************************************
		OPSYS_PROJ2
**************************************************
**************************************************


**************************************************
		    MEMBERS
**************************************************

Member 1: Dillon Prendergast
Member 2: Zachary Timmerman
Member 3: John Thomas Lascha

**************************************************
		DIVISION OF LABOR
**************************************************

Dillon Prendergast
  -project manager
  -README
  -part1
  -part2
  -part3
	-syscalls
	-proc file output
	-debugging

Zachary Timmerman
  -Part1
  -part2
  -part3
	-syscalls
	-debugging
  -README.txt

John Thomas Lascha
  -part1
  -part2
  -part3
	-installing kernel
	-elevator implementation
	-debugging
**************************************************
		TAR ARCHIVE CONTENTS
**************************************************

project2_lascha_prendergast_timmerman.tar contents:

README.txt
elevator directory:
	-Makefile
	-elevator.c		//main program with implemented functions
	-elevator_calls.c	//linking of syscalls
part2 directory:
	-my_xtime
	-Makefile
part1 directory:
	-part1.c

**************************************************
		   COMPILATION
**************************************************

Completed Using:
Linux 4.14.12 x86_64
gcc version 5.4.0

**************************************************
		   THE MAKEFILE
**************************************************

Part1:

To make:
$> gcc -o part1.x part1.c

To run:
$> strace -o log ./part1.x

my_xtime:

To make:
$> make my_xtime.c

To insert module:
$> sudo insmod my_xtime.ko

To view proc file:
$> cat /proc/timed

To remove module:
$> sudo rmmod my_xtime

To clean directory:
$> make clean

Elevator:

To build:
$> make

To install module:
$> sudo insmod elevator.ko

To view proc file:
$> cat /proc/elevator

To remove module:
$> sudo rmmod elevator

To clean directory:
$> make clean

**************************************************
		      BUGS
**************************************************

-If elevator is started then immediately closed without
 letting on any passengers, system shows unintended behavior
 and module is removed.

**************************************************
		UNFINISHED PORTIONS
**************************************************

-Optimizing elevator for fastest behavior

**************************************************
		     COMMENTS
**************************************************
