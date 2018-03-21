#include <sys/syscall.h>
#define _START 333
#define _REQUEST 334
#define _STOP 335

int main(){

  syscall(333);
  syscall(334,3,3,3);
  syscall(335);

  return 0;
}
