#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"

int get_num_products(char * file_name);

void free_linked(order_struct *root);

void free_all();

struct trader_struct get_trader_epoll(struct epoll_event ev);



extern void sig_handler(int sig, siginfo_t *info, void *ucontext);

extern void child_term_handler(int sig, siginfo_t *info, void *ucontext);

extern void create_pipe_read(struct trader_struct *trader, char * temp_exch_name, char * temp_trad_name);

extern void open_pipes(struct trader_struct *trader, struct epoll_event *ev);

extern void check_signals();

extern void invalid_order(struct trader_struct *trader);

extern void amend();

extern void cancel_order();

extern int write_trader(struct trader_struct *trader, char *msg);

extern void process_order(char * order_type, int current_order_id, char * product, int qty, int price, int trader_id);



#endif
