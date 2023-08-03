#ifndef PIPE_RELATED_H
#define PIPE_RELATED_H
#include "pe_common.h"


void create_pipe_read(struct trader_struct *trader, char * temp_exch_name, char * temp_trad_name);

int write_trader(struct trader_struct *trader, char *msg);

void write_all_traders(char *msg, int ID);

void open_pipes(struct trader_struct *trader, struct epoll_event *ev);

#endif