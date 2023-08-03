#ifndef PE_COMMON_H
#define PE_COMMON_H

//#define _POSIX_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#define FIFO_EXCHANGE "/tmp/pe_exchange_%d"
#define FIFO_TRADER "/tmp/pe_trader_%d"
#define FEE_PERCENTAGE 1

#define LOG_PREFIX "[PEX]"

#define MAX_FILE_LENGTH 25
#define BUFFER_SIZE 50
#define PIPE_NAME_LENGTH 25

#define TRUE 1
#define FALSE 0
#define SIGNAL_VOLUME 15

extern int epoll_fd;
extern char file_products[20];
extern struct epoll_event *epoll_tracked;
extern struct epoll_event *events;
extern struct trader_struct *traders;
extern struct product_struct *products;

extern int glob_argc;
extern int trader_num;
extern int total_fees;
// pid_t *disconnect;
// int disconnect_counter = 0;
extern pid_t signal_rec[SIGNAL_VOLUME];
extern int signal_counter;
extern pid_t disconnect[SIGNAL_VOLUME];
extern int disconnect_counter;
extern int sigusr1_rec;

typedef struct trader_product_struct{
    char name[BUFFER_SIZE]; //the name of the product.
    int qty; //the quantity of this struct that is owned. 
    int balance; //the balance pertaining to this product. 
} trader_product_struct;

typedef struct order_struct{
    char type[BUFFER_SIZE]; //the type of order buy, sell etc
    char product[BUFFER_SIZE]; //the product name
    int id; //the id of the order
    int qty; //the quantity
    int price;
    int traderID; //id of the trader who made the order.
    struct order_struct * next;
} order_struct;

typedef struct product_struct{
    char name[BUFFER_SIZE];
    int buy_levels;
    int sell_levels;
    struct order_struct * orders_root;
}product_struct;

typedef struct trader_struct{
    int cash; //this is for the current balance the trader
    int orderId; //this is to keep a tally of how many order that this trader has made 
    int ID; //the order in which this specific trader was activated (used for the pipe creation later on).
    int formerID; //this is for an inactive trader (so that we know what the id was when it was active.)
    pid_t PID; //the processor id of this trader
    int exchfd; //exchange write pipe fd. 
    int tradfd; //trader write pipe fd.
    int totalFees; //how many fees earned from this trader.
    char file[MAX_FILE_LENGTH]; //this is the name of the binary file associated with this trader.
    //these next two are the actual names of the pipes. 
    char exch_name[PIPE_NAME_LENGTH];
    char trad_name[PIPE_NAME_LENGTH];
    struct epoll_event *ev;
    struct trader_product_struct *trader_product_struct;

}trader_struct;

#endif
