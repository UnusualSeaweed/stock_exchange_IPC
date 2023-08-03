#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define FIFO_EXCHANGE "/tmp/pe_exchange_0"
#define FIFO_TRADER "/tmp/pe_trader_0"


int main(){
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
    char buf[50];
    buf[49] = '\0';
    if(read(exch_fd, buf, sizeof(buf)) == -1) exit(EXIT_FAILURE);
    for(int i = 0; i < 50; i++){
        if(buf[i] == ';'){
            buf[i+1] = '\0';
            break;
        }
    }

    // if(strcmp("MARKET OPEN",buf) != 0){
    //     printf("incorrect output\n");
    // }

}