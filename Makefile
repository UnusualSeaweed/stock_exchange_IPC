CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address,leak
SRC=pe_exchange.c pe_trader.c my_signals.c orderbook.c pipe_related.c
LDFLAGS=-lm
BINARIES=pe_exchange pe_trader
OBJ=$(SRC:.c=.o)

all: $(BINARIES)

test:
	bash test1.sh

.PHONY: clean
clean:
	rm -f $(BINARIES)

pe_exchange.o: pe_exchange.c pe_exchange.h
	$(CC) $(CFLAGS) -c $<

pipe_related.o: pipe_related.c pipe_related.h
	$(CC) $(CFLAGS) -c $<

my_signals.o: my_signals.c my_signals.h
	$(CC) $(CFLAGS) -c $<

orderbook.o: orderbook.c orderbook.h
	$(CC) $(CFLAGS) -c $<

# .c.o: this was a failure, time to do them one by one.
# 	 $(CC) $(CFLAGS) $<

pe_exchange: pe_exchange.o pipe_related.o my_signals.o orderbook.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

pe_trader: pe_trader.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)





