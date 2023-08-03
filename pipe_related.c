
#include "pipe_related.h"



//created the pipes to read to.
void create_pipe_read(struct trader_struct *trader, char * temp_exch_name, char * temp_trad_name){ 
	// "/tmp/pe_exchange_<Trader ID>" pipe that exchange will write to. 
	// "/tmp/pe_trader_<Trader ID>" pipe that trader will write to.
	sprintf(temp_exch_name, FIFO_EXCHANGE, trader->ID);
	sprintf(temp_trad_name, FIFO_TRADER, trader->ID);

	// printf("inside create_pipe_ read: trader.exch_name: %s\n", temp_exch_name);

	//creating named pipes
	if(mkfifo(temp_exch_name, 0777) == -1){
		if(errno != EEXIST){
			perror("exchange pipe failed creation\n");
			exit(EXIT_FAILURE);
		}

	}
	printf("%s Created FIFO %s\n",LOG_PREFIX, temp_exch_name);

	if(mkfifo(temp_trad_name, 0777) == -1){
		if(errno != EEXIST){
			perror("exchange pipe failed creation\n");
			exit(EXIT_FAILURE);
		}
	}

	printf("%s Created FIFO %s\n",LOG_PREFIX, temp_trad_name);
	
}

//given the trader id as input, send a message to pipe.
int write_trader(struct trader_struct *trader, char *msg){ 
	int write_result = write(trader->exchfd, msg, strlen(msg));
	if(write_result == -1){
		perror("failed to write");
		exit(EXIT_FAILURE);
	}
	//sending signal. 
	kill(trader->PID,SIGUSR1);
	return 0;
}

//sends an order to all traders except to the ID specified in 'ID' attribute. 
void write_all_traders(char *msg, int ID){ 
	for(int i = 0; i < glob_argc; i++){
		if(traders[i].ID != -1 && traders[i].ID != ID){ //checking if it is an active trader.
			write_trader(&traders[i], msg);
		}
	}
}

//opens the pipes
void open_pipes(struct trader_struct *trader, struct epoll_event *ev){
	trader->exchfd = open(trader->exch_name, O_WRONLY);
	printf("%s Connected to %s\n",LOG_PREFIX, trader->exch_name);
	trader->tradfd = open(trader->trad_name, O_RDONLY);
	printf("%s Connected to %s\n",LOG_PREFIX, trader->trad_name);

	//checking to see if pipes were made correctly. 
	if(trader->tradfd == -1 || trader->exchfd == -1){
		perror("pipe creation error");
		exit(EXIT_FAILURE);
	}
	char market_open[] = "MARKET OPEN;"; //check this is the right message.
	write_trader(trader,market_open);

	ev->data.fd = trader->tradfd; //passing fd to struct
	ev->events = EPOLLHUP; //this is so that the pipes can be monitored for closing (in the instance that their is no signal sent).

	//officially adding the struct for tracking and checking if anything went wrong. 
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, trader->tradfd, ev) == -1){
		perror("error tracking ev");
		exit(EXIT_FAILURE);
	}

}