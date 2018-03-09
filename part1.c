#include <stdio.h>

int main(){
FILE *fp;
for(int i=0;i<3;i++){
fp=fopen("file","w");
fclose(fp);
}
}
