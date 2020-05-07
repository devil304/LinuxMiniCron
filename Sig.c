#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

volatile bool signalRec;

void handler(int signum){
    printf("signal!\n");
	signalRec = false;
    exit(signum);
}

int main(void){
	signalRec = true;
    if(signal(SIGQUIT, handler)==SIG_ERR){
		perror("signal");
	}
    while(signalRec)
    {
        printf("working...\n");
        sleep(1);
    }
    return 0;
