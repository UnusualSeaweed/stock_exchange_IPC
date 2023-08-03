#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "pe_common.h"

void print_list_orders(order_struct *root);

int duplicate_orders(product_struct *product, char * order_type);

void print_out_products();

void print_out_positions();

void order_book();

void add_order(order_struct *root, order_struct *node);

struct trader_struct * find_trader_withID(int ID);

void remove_free_order(product_struct * product, order_struct *order);

void adjust_trader_balance(struct trader_struct *trader, int qty, int balance, order_struct *order);

int check_matched(product_struct * product, order_struct *order);

void process_order(char * order_type, int current_order_id, char * product, int qty, int price, int trader_id);

void invalid_order(struct trader_struct *trader);

void amend();

void cancel_order();

extern int get_num_products(char * file_name);

extern int write_trader(struct trader_struct *trader, char *msg);

extern void write_all_traders(char *msg, int ID);

#endif
