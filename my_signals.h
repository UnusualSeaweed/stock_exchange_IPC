#ifndef MY_SIGNALS_H
#define MY_SIGNALS_H

#include "pe_common.h"

void sig_handler(int sig, siginfo_t *info, void *ucontext);

void child_term_handler(int sig, siginfo_t *info, void *ucontext);

void check_signals();

extern void process_order(char * order_type, int current_order_id, char * product, int qty, int price, int trader_id);

extern void amend();

extern void cancel_order();

extern int write_trader(struct trader_struct *trader, char *msg);

extern void invalid_order(struct trader_struct *trader);

extern int get_num_products(char * file_name);

#endif