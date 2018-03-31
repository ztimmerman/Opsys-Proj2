#include <unistd.h>
#define _START 333
#define _REQUEST 334
#define _STOP 335

int main(){

  syscall(333);
  syscall(334,1,1,6);
  syscall(334,1,10,1);
  syscall(334,1,10,1);
  syscall(334,1,9,1);
  syscall(334,1,8,1);
  syscall(334,1,5,1);
//sleep(20);
//  syscall(335);

  return 0;
}
