#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#define FIFO_EXCHANGE "/tmp/pe_exchange_0"
#define FIFO_TRADER "/tmp/pe_trader_0"


int main(){
    fprintf(stderr,"goodness\n");
    //connection to first exchange pipe read only.
    sleep(2);
    //setting up pipes.
    char exch_name[] = FIFO_EXCHANGE;
    int exch_fd = open(exch_name, O_RDONLY);
    if(exch_fd == -1){
        if(errno != EEXIST){
            perror("pipe issue trader reading exchange");
            return 1;
        }

    }

    //connection to trader pipe write only. 
    char trad_name[] = FIFO_TRADER; 
    int trad_fd = open(trad_name, O_WRONLY);
    if(trad_fd == -1){
        if(errno != EEXIST){
            perror("pipe issue trader writing exchange");
            return 2;
        }
    }  

    // sleep(1);
    fprintf(stderr,"about to write\n");
    // kill(getppid(),SIGUSR1);
    char msg[] = "BUY 0 GPU 30 500;";
    if(write(trad_fd, msg, strlen(msg)) == -1){
        perror("what is this??");
        fprintf(stderr,"not good\n");
    }
    fprintf(stderr,"about to send signal\n");
    if(kill(getppid(),SIGUSR1) < 0){
        perror("kill");
    }
    fprintf(stderr,"signal sent\n");


    // sleep(10);
    // if(strcmp("MARKET OPEN",buf) != 0){
    //     write(1,"incorrect output\n",strlen("incorrect output\n"));
    // }

    close(trad_fd);
    close(exch_fd);

}