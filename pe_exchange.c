/**
 * comp2017 - assignment 3
 * Daniel Borg
 * dbor3200
 */

#include "pe_exchange.h"

int epoll_fd;
char file_products[20];
struct epoll_event *epoll_tracked;
struct epoll_event *events;
struct trader_struct *traders;
struct product_struct *products;


int glob_argc;
int trader_num;
int total_fees;

pid_t signal_rec[SIGNAL_VOLUME] = {0};
int signal_counter = -1;

pid_t disconnect[SIGNAL_VOLUME] = {0};
int disconnect_counter = -1;
int sigusr1_rec;

// gets the number of products from the file. 
int get_num_products(char * file_name){
	FILE *file = fopen(file_name, "r");
	char quantity[3];
	fgets(quantity, 3, file);
	fclose(file);
	return atoi(quantity);
}

//frees an entire linked list.
void free_linked(order_struct *root){
	order_struct *curr = root;
    order_struct *behind = NULL;
	while(curr != NULL){
		//free(behind);
		behind = curr;
		curr = curr->next;
		free(behind);
	}
}

//frees everything. 
void free_all(){
	for(int i = 0; i < glob_argc; i++){
		free(traders[i].trader_product_struct);
	}
	free(traders);
	free(epoll_tracked);
	free(events);
	close(epoll_fd);
	for(int i = 0; i < get_num_products(file_products); i++){
		free_linked(products[i].orders_root);
	}
	free(products);
}

//identifies the trader using epoll attribute in struct. 
struct trader_struct get_trader_epoll(struct epoll_event ev){ //returning the associated trader from the epoll event.
	for(int i = 0; i < glob_argc; i++){
		if(traders[i].tradfd == ev.data.fd){
			return traders[i];
		}
	}
	return traders[0]; 
}

//checks if there are any active traders still in play.
int check_finished(){
	if(trader_num == 0){
		printf("%s Trading completed\n", LOG_PREFIX);
		printf("%s Exchange fees collected: $%d\n", LOG_PREFIX, total_fees);
		free_all();
		exit(0);
	}
	return -1; //if there are still traders active. 
}

//remove trader from market. 
void remove_trader(pid_t PID){
	//idenfify the trader
	if(PID == 0){
		return;
	}
	int i = 0;
	for(; i < trader_num; i++){
		if(traders[i].PID == PID){
			break;
		}
	}

	if(traders[i].ID == -1){ //checking if it is an active trader. 
		return;
	}

	//closing all pipes regarding that trader
	// when there are no more active traders there will be no pipes open. 
	unlink(traders[i].exch_name);
	unlink(traders[i].trad_name);

	close(traders[i].exchfd);
	close(traders[i].tradfd);


	trader_num -= 1; //one less active trader. 
	//collecting total fees collected from that trader.
	total_fees += traders[i].totalFees;
	printf("%s Trader %d disconnected\n",LOG_PREFIX,traders[i].ID);
	traders[i].ID = -1; //trader no longer active.

	//remove from epoll
	epoll_ctl(epoll_fd,EPOLL_CTL_DEL,traders[i].tradfd, traders[i].ev);
	check_finished(); //see if there are any other active traders, otherwise close the program. 
}

//turns an int into a char.
char * int_to_char(int num, char temp[MAX_FILE_LENGTH]){
	sprintf(temp, "%d",num);
	return temp;
}

//read the products from the product file. 
void read_products(char * file_name){
	FILE *file = fopen(file_name, "r");
	char quantity[3];
	fgets(quantity, 3, file);
	int num = atoi(quantity);
	//stores all products into malloc. 
	products = (struct product_struct*) calloc(num, sizeof(struct product_struct));

	//print out products that are being traded.
	printf("%s Trading %d products: ", LOG_PREFIX, num);
	for(int i = 0; i < num; i++){

		fgets(products[i].name, BUFFER_SIZE, file);
		products[i].name[strlen(products[i].name) - 1] = '\0';
		printf("%s", products[i].name);
		if(i != num -1){
			printf(" ");
		}

		//initialize trader struct for every product being
		products[i].buy_levels = 0;
		products[i].sell_levels = 0;
		products[i].orders_root = (order_struct *) calloc(1,sizeof(order_struct));
		products[i].orders_root->price = INT_MAX;


	}
	printf("\n");
}

int main(int argc, char **argv) {
	glob_argc = argc;
	trader_num = argc - 2;
	printf("%s Starting\n", LOG_PREFIX);
	strcpy(file_products, argv[1]);
	read_products(argv[1]);

	//preparing signal handlers. 
	struct sigaction sa = {0};
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = &sig_handler;
	sigaction(SIGUSR1, &sa, NULL);

	struct sigaction child_handle = {0};
	child_handle.sa_flags = SA_SIGINFO;
	child_handle.sa_sigaction = &child_term_handler;
	sigaction(SIGCHLD, &child_handle, NULL);


	//creating epoll instance.
	epoll_fd = epoll_create1(0); //start an epoll instance (see also epoll_creat1())
	if(epoll_fd == -1){ //error checking, see man epoll
        perror("error creating epoll");
        exit(EXIT_FAILURE);
    }
	traders = (struct trader_struct*) calloc(argc, sizeof(struct trader_struct)); //for storing all the traders inside.
	for(int i = 0; i < argc; i++){
		traders[i].ID = -1; //representing a trader that isn't in use. 
		traders[i].tradfd = -1;
		traders[i].formerID = -1;
		traders[i].orderId = 0;
		//this set up a variable for every product that is being traded on the market. 
		traders[i].trader_product_struct = (trader_product_struct *) calloc(get_num_products(file_products), sizeof(trader_product_struct));
		for(int j = 0; j < get_num_products(file_products); j++){
			//copying all of the traders names into the trader's individual product arrays.
			strcpy(traders[i].trader_product_struct[j].name, products[j].name);
		}
	}

	//preparing epoll_events to track by the epoll. 
	epoll_tracked = (struct epoll_event*) calloc(argc, sizeof(struct epoll_event)); //these are for the specific epoll 
	events = (struct epoll_event*) calloc(argc, sizeof(struct epoll_event));  //this is for epoll wait function.
	//initializing all of the traders. 
	for(int i = 2; i < argc; i++){
		//putting all the relevant information into each trader. index is i-2 due to their being a product file and an exchange file as the first
		//two command line arguments. 
		strncpy(traders[i - 2].file, argv[i], MAX_FILE_LENGTH); //putting the file name into the struct. 
		traders[i - 2].ID = i - 2; //putting the traderID into the struct. 
		traders[i - 2].formerID = i - 2; 
		epoll_tracked[i-2].data.u32 = i-2; //storing the index of the trader for quick access later see get_trader_epoll
		epoll_tracked[i-2].data.u64 = i-2;
		traders[i-2].ev = &epoll_tracked[i-2];

		//creating pipes part. 
		char temp_exch_name[PIPE_NAME_LENGTH];
		char temp_trad_name[PIPE_NAME_LENGTH];
		create_pipe_read(&traders[i-2], temp_exch_name, temp_trad_name); //create the pipes for each trader.
		strncpy(traders[i-2].exch_name, temp_exch_name, PIPE_NAME_LENGTH); //pipe names being stored into the struct. 
		strncpy(traders[i-2].trad_name, temp_trad_name, PIPE_NAME_LENGTH);

		//forking.
		traders[i-2].PID = fork();
		
		if(traders[i-2].PID == -1){
			perror("issue forking");
			exit(EXIT_FAILURE);
		}
		else if(traders[i-2].PID == 0){
			//child process 
			//launching trader executable.
			// printf("%s Starting trader %d (%s)\n", LOG_PREFIX, traders[i-2].ID ,traders[i-2].file);
			char temp[MAX_FILE_LENGTH];
			if(execl(traders[i-2].file, traders[i-2].file, int_to_char(traders[i-2].ID,temp), (char *) NULL) == -1){
				perror("execl issue");
				exit(EXIT_FAILURE);
			} 
		}else{
			printf("%s Starting trader %d (%s)\n", LOG_PREFIX, traders[i-2].ID ,traders[i-2].file);
			//opening pipes for that trader. 
			open_pipes(&traders[i-2], &epoll_tracked[i-2]);
		}
	}

	while(1){
		//awaiting signal.
		pause();

		//process signals "SIGUSR1" get priority.
		check_signals();

		//this is to check for any sigchild signals. 
		if(disconnect_counter != -1){
			int temp_counter = disconnect_counter + 1;
			while(1){
				if(temp_counter == disconnect_counter && disconnect[temp_counter] == 0){
					break;
				}
				if(temp_counter == SIGNAL_VOLUME){
					temp_counter = 0;
				}
				if(disconnect[temp_counter] == 0){
					temp_counter += 1;
					continue;
				}
				remove_trader(disconnect[temp_counter]);
				disconnect[temp_counter] = 0;
			}
		}
	//==========================================================================
		//this is a contingency for child signals, (in the instance that one is not send by the trader or it is lost).
		int nfds = epoll_wait(epoll_fd, events, glob_argc, 40);
		if(nfds == -1){
			//this is if the epoll_wait gets interrupted by a signal.
			if(errno == EINTR){
				continue;
			}
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}
		//remove all the traders that get the 
		for(int i = 0; i < nfds; i++){
			remove_trader(get_trader_epoll(events[i]).PID);
		}
	}

	//this code should never be executed.
	//free everything just in case.
	free(traders);
	free(epoll_tracked);
	close(epoll_fd);
	printf("ending\n");
return 0;
}
