// #include "pe_trader.h"

// int main(int argc, char ** argv) {
// 	// implement your trader program to be fault-tolerant.
// }


//==========================================================
#include "pe_trader.h"
#define TRUE 1
#define FALSE 0
#define ORDER_SIZE 30 //subject to change

struct order{
    char* type;
    char* product;
    int qty;
    int price;
} current_order;

int exch_fd;
int trad_fd;
int is_open;
int order_id;
int die;
int accepted;
char buffer[BUFFER_SIZE + 1];
int told;

//when received an order this function will send the message to the exchange to write the signal.
//mindlessly buys everything. 
int order_logic(){
    char output[ORDER_SIZE];
    if(strcmp(current_order.type, "SELL") == 0){
        sprintf(output, "BUY %d %s %d %d;",
         order_id++, current_order.product, current_order.qty,
          current_order.price);
        if(write(trad_fd, output, strlen(output)) == -1) perror("order_logic issue");
        kill(getppid(),SIGUSR1);
    }
    return 0; 
}

//parses the orders.
int order_parser(char input[ORDER_SIZE+1]){
    current_order.product = 0;
    current_order.type = 0;
    strtok(input," "); //market

    current_order.type = strtok(NULL, " "); //type

    //checking to see if it is the 'MARKET OPEN;' message.
    if(strncmp(current_order.type, "OPEN;", sizeof("OPEN;")) == 0){
        is_open = TRUE;
        return 1;
    }


    current_order.product = strtok(NULL, " "); //product

    char * qty = strtok(NULL, " "); //quantity from message
    if(qty == NULL) return 1;
    current_order.qty = atoi(qty);

    char * price = strtok(NULL,";"); //price from message
    if(price == NULL) return 1;
    current_order.price = atoi(price);

    //as per spec, auto-trader will not trade if there is an order with quanity over 1000
    if(current_order.qty >= 1000){ 
        die = TRUE;
        return 0;
    }
    order_logic();
    return 1;
}

//reads the pipe for a message.
void reading(char from_exchange[ORDER_SIZE]){
    char single_char;
    for(int i = 0; i < ORDER_SIZE; i++){
        if(read(exch_fd, &single_char, sizeof(char)) == -1) return;
        if((single_char <= 126 && single_char >= 33) || single_char == 32){
            strncat(from_exchange, &single_char, 1);
            if(single_char == ';'){
                break;
            }
        }
    }
}


void acting(){
    told -= 1;
    char from_exchange[ORDER_SIZE+1] = {0};
    from_exchange[ORDER_SIZE] = '\0';
    reading(from_exchange);
    char str_copy[ORDER_SIZE +1];
    strncpy(str_copy,from_exchange, ORDER_SIZE); 
    str_copy[ORDER_SIZE] = '\0';
    char * first = strtok(str_copy, " ");

    if(strcmp(first,"ACCEPTED") == 0){
        accepted = TRUE; 

        return;
    } 
    else if(strcmp(first,"FILL") == 0){

        return;
    } 
    else if(strcmp(first, "MARKET") == 0) order_parser(from_exchange);
}

//signal handler. 
void handler(){
    told += 1;
}


int main(int argc, char ** argv) {
    told = 0; //inital amount of signals is 0.
    int selfid = atoi(argv[1]); 
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }
    is_open = FALSE;
    die = FALSE;
    accepted = FALSE;
    order_id = 0;
    //preparing signal handler.
    struct sigaction sa = {0};
    sa.sa_handler = &handler;
    sigaction(SIGUSR1, &sa, NULL);
    // connect to named pipes
    char exch_name[25]; 
    char trad_name[25];
    sprintf(exch_name, FIFO_EXCHANGE, selfid);
    sprintf(trad_name, FIFO_TRADER, selfid);
    //connection to first exchange pipe read only.
    exch_fd = open(exch_name, O_RDONLY);
    if(exch_fd == -1){
        perror("pipe issue trader reading exchange");
        return 1;
    }

    //connection to trader pipe write only. 
    trad_fd = open(trad_name, O_WRONLY);
    if(trad_fd == -1){
        perror("pipe issue trader writing exchange");
        return 2;
    }  

    pause(); // awaiting for market to be open.
        while(TRUE){
            accepted = FALSE;
            if(told > 0){
                acting();
            }
            if(die == TRUE){ //in this instance the trader needs to close. 
                close(exch_fd);
                close(trad_fd); 
                break;
            }

        }

        accepted = FALSE;

    //close all traders before closing. 
    close(exch_fd);
    close(trad_fd);

    
}