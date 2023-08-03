//used for signal handling and the processing of signals. (there is a bit more signal processing in the pe_exchange.c - with epoll)
#include "my_signals.h"


//this is for SIG_USR1 signals. 
void sig_handler(int sig, siginfo_t *info, void *ucontext){
	signal_counter += 1; //increases by 1 in order to get the next free spot in the signal_rec array. 
	if(signal_counter == SIGNAL_VOLUME){
		signal_counter = 0;
	}
	signal_rec[signal_counter] = info->si_pid; //works using a makeshift priority queue out of a single array. 
	// Note: this type of priority queue has a chance of overwriting the oldest signals if they haven't been processed yet. 
    
}

//same as above but specifically for sig_child signals. 
void child_term_handler(int sig, siginfo_t *info, void *ucontext){ 
	disconnect_counter += 1; //incrementing signal counter by one. 
	if(disconnect_counter == SIGNAL_VOLUME){
		disconnect_counter = 0;
	}
	disconnect[disconnect_counter] = info->si_pid; //placing it into the makeshift priority queue. 
	// Note: this type of priority queue has a chance of overwriting the oldest signals if they haven't been processed yet. 
}

//checks signals (only of type sigusr1).
void check_signals(){
	char order_type[17], product[17];  //the size of these variables is as determined by the assignment specification. 
	order_type[16] = '\0';
	product[16] = '\0';
	struct trader_struct the_trader;
	int temp_counter;
	if(signal_counter == -1){ //no signals have been sent yet
		return;
	}
	temp_counter = signal_counter + 1;
	while(TRUE){
		//this means that the array has encountered a 0 in the array (hence no signal) 
		//and hence there is no more signals to process. (the signal counter is back to the start)
		if(temp_counter == signal_counter && signal_rec[temp_counter] == 0){ 
			break;
		} 
		if(temp_counter == SIGNAL_VOLUME){
			temp_counter = 0;
		}
		//iterating through the entire array to make sure there are no other signals in there. 
		if(signal_rec[temp_counter] == 0){
			temp_counter += 1;
			continue;
		}
		pid_t pid = signal_rec[temp_counter];
		signal_rec[temp_counter] = 0;
		char buf[BUFFER_SIZE] = {0};
		char bufbuf;
		buf[BUFFER_SIZE - 1] = '\0';
		int trader_index;
		for(int i = 0; i < glob_argc; i++){
			int read_result;
			if(pid == traders[i].PID){ //identifying the trader. 
				the_trader = traders[i];
				trader_index = i;
				for(int j = 0; j < BUFFER_SIZE - 2; j++){
					read_result = read(traders[i].tradfd, &bufbuf, 1); //reading from pipe on char at a time. 
					buf[j] = bufbuf;
					if(bufbuf == ';'){ //stopping at semicolon.
						buf[j] = '\0';
						break;
					}
					if(read_result == -1){ 
						perror("read issue");
						exit(EXIT_FAILURE);
					}
				}

				printf("%s [T%d] Parsing command: <%s>\n",LOG_PREFIX, the_trader.ID,buf);
				

				if(read_result == -1){ 
					perror("read issue");
					exit(EXIT_FAILURE);
				}
				break;
			}
		}
		int word_counter = 0;
		for(int i = 0; i < strlen(buf); i++){ //counting the words in the input by counting the spaces. 
			if(buf[i] == ' '){
				word_counter += 1;
			}
		}

		int curr_id, qty, price;

		//parsing all the input from 'buf' into the respective variable types. 
		sscanf(buf, "%s %d %s %d %d", order_type, &curr_id, product, &qty, &price);
		if(curr_id != the_trader.orderId){
			invalid_order(&the_trader);
			return;
		}


		if(strcmp(order_type,"AMEND") == 0){
			amend();
			return;
		}
		
		if(strcmp(order_type,"CANCEL") == 0){
			cancel_order(trader_index);
			return;
		}

		traders[trader_index].orderId += 1; // keeping count of all the orders that came from the trader. 

		//the following if statements are all simply different checks to make sure that all of the values are inside the accepted limits. 
		if(word_counter != 4){
			invalid_order(&the_trader); 
			return;
		}

		if(qty > 999999){
			invalid_order(&the_trader); 
			return;
		}

		if(price > 999999){
			invalid_order(&the_trader); 
			return;
		}

		if(the_trader.ID > 999999){
			invalid_order(&the_trader); 
			return;
		}

		if(qty <= 0){
			invalid_order(&the_trader); 
			return;
		}

		if(price <= 0){
			invalid_order(&the_trader); 
			return;
		}

		if(the_trader.ID < 0){
			invalid_order(&the_trader); 
			return;
		}

		for(int i = 0; i < get_num_products(file_products); i++){ //making sure that it is a recognised product. 
			if(strcmp(products[i].name,product) == 0){
				break;
			}
			if(i == get_num_products(file_products) -1){
				invalid_order(&the_trader);
				return;
			}
		}

		char msg[18];
		msg[17] = '\0';
		//writing the accepted message to the trader that send the order. 
		sprintf(msg, "ACCEPTED %d;", curr_id);
		write_trader(&the_trader, msg);
		//do further ordering processing. 
		process_order(order_type, curr_id, product, qty, price, the_trader.ID);
	}
}