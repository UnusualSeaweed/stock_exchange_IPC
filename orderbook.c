
#include "orderbook.h"

//print the list orders. 
void print_list_orders(order_struct *root){
    order_struct *temp = root->next;
	order_struct *behind = root;
	int counter = 1;
	int qty = 0;
	while(behind != NULL && temp != NULL){
		//they are the same type of order.
		if(strcmp(behind->type,temp->type) == 0 && //same order type
			behind->price == temp->price){ //same price.
			counter += 1;
			qty += behind->qty;
		}else{//they are different types.
			if(behind != root){ //(don't want to print out the root node).
				qty += behind->qty;
				//following if and else is used to determine whether or not to use plural for 'orders'. 
				if(counter > 1){
					printf("%s		%s %d @ $%d (%d orders)\n", LOG_PREFIX, behind->type, qty, behind->price, counter);
				}else{
					printf("%s		%s %d @ $%d (%d order)\n", LOG_PREFIX, behind->type, qty, behind->price, counter);
				}
				counter = 1;
				qty = 0;
				}
			}
		//iteration.
		behind = temp;
		temp = temp->next;
	}
	if(behind != root){
		qty += behind->qty;
		if(counter > 1){
			printf("%s		%s %d @ $%d (%d orders)\n", LOG_PREFIX, behind->type, qty, behind->price, counter);
		}else{
			printf("%s		%s %d @ $%d (%d order)\n", LOG_PREFIX, behind->type, qty, behind->price, counter);
		} 
		return;				
	}
}



// this function is to determine the buy levels and sell levels of a particular product. (used for printing)
int duplicate_orders(product_struct *product, char * order_type){ //order type is buy/sell etc. 
	order_struct *behind = product->orders_root;
	order_struct *curr = behind->next;
	int levels = 0;
	while(curr != NULL){
		// In order to identify a duplicate order, the order type and order price must be the same. 
		if(strcmp(order_type,curr->type) == 0 && strcmp(order_type,curr->type) == 0){ //making sure that it is the correct order type.
			if(behind->price != curr->price){ //price needs to be the same.
				levels += 1;
			}
		}
		//iteration 
		behind = curr;
		curr = curr->next;
	}
	return levels;

}

//simply printing out the products for each of the products for the orderbook.
void print_out_products(){
	for(int i = 0; i < get_num_products(file_products); i++){
		//count duplicate buy and sell orders
		int buy = duplicate_orders(&products[i], "BUY");
		int sell = duplicate_orders(&products[i], "SELL");
		
		printf("%s	Product: %s; Buy levels: %d; Sell levels: %d\n",LOG_PREFIX, products[i].name, buy, sell);

		//printing orders specific to that product.
		print_list_orders(products[i].orders_root);
	}
}

// prints out the positions. 
void print_out_positions(){
	for(int i = 0; i < glob_argc; i++){
		if(traders[i].formerID != -1){ //making sure that the trader is an active trader. 
			printf("%s	Trader %d: ",LOG_PREFIX, traders[i].formerID);
			for(int j = 0; j < get_num_products(file_products); j++){ //printing all the product positions specific to this trader. 
				printf("%s %d ($%d)", traders[i].trader_product_struct[j].name,
				traders[i].trader_product_struct[j].qty, traders[i].trader_product_struct[j].balance);
				if(j != get_num_products(file_products) - 1){
					printf(", ");
				}
			}
			printf("\n");
		}
	}
}

//simply prints out the titles before calling other functions. 
void order_book(){
	printf("%s	--ORDERBOOK--\n", LOG_PREFIX);
	print_out_products();

	printf("%s	--POSITIONS--\n", LOG_PREFIX);
	print_out_positions();
}

//adds an order into the products list of orders in order of price (highest price first).
void add_order(order_struct *root, order_struct *node){
    order_struct *curr = root->next; //curr or current is the current node being iterated over. 
    order_struct *behind = root; //this is one node behind the current node being iterated over. 
    while(curr != NULL){
        if(node->price > curr->price){ //if the current node value is bigger than the node we are adding's value
        //we finally know the position of where to add it. 
            node->next = curr; //setting current node to be the next node.
            behind->next = node; //setting the added node to the previous nodes next. 
            break;
        }
        behind = curr;
        curr = curr->next;
    }
    //must be smaller than the smallest. make sure that curr is the last node before you add it to the end.
    if(curr == NULL){
        node->next = curr; //setting current node to be the next node.
        behind->next = node; //setting the added node to the previous nodes next. 
        return;
    }	
}

//find trader struct using ID.
struct trader_struct * find_trader_withID(int ID){
	for(int i = 0; i < glob_argc; i++){
		if(traders[i].formerID == ID){
			return &traders[i];
		}
	}
	return NULL;
}

//removes an order out of the linked list and frees it also. 
void remove_free_order(product_struct * product, order_struct *order){
	order_struct * behind = product->orders_root; 
	order_struct * curr = behind->next;
	while(curr != NULL){
		if(order->traderID == curr->traderID && order->id == curr->id){
			behind->next = curr->next;
			free(order);
			return;
		}

		//iteration
		behind = curr;
		curr = curr->next;
	}

}

//puts the quantity and balance changes into the trader struct. 
void adjust_trader_balance(struct trader_struct *trader, int qty, int balance, order_struct *order){
	for(int i = 0; i < get_num_products(file_products); i++){ 
		if(strcmp(trader->trader_product_struct[i].name,order->product) == 0){
			trader->trader_product_struct[i].qty += qty;
			trader->trader_product_struct[i].balance += balance;
			return;
		}
	}
}

//this is the matching function. performs the price-time algorithm to match orders together. 
int check_matched(product_struct * product, order_struct *order){ //returns one if remaining quantity is >0.
	order_struct * curr = product->orders_root->next;
	order_struct * temp = NULL;
	int value; //the dollar value made from that particular trade.
	int fee; //the fee taken from the transaction. 
	int qty; //the amount that the order is being used up by. 
	int freed = FALSE;
	while(curr != NULL){
		//check that one is a sell and one is a buy. must be opposites if they are to be considered for a match.
		if(strcmp(order->type,curr->type) != 0){ 
			if(strcmp(order->type, "SELL") == 0 && order->price <= curr->price){ 
				// in the case that it is a 'sell' order.
				// price of the sell order needs to be lower or equal to qualify as a match.  
				if(order->qty > curr->qty){ //order has a higher quantity than the current node. 
					qty = curr->qty;
					order->qty -= qty;
					curr->qty = 0; 
				}else{ //the order has a lower quantity than the current node.
					qty = order->qty;
					curr->qty -= qty; 
					order->qty = 0;
				}
					value = curr->price * qty;
					fee = round(value * 0.01); //fee calculation.
					total_fees += fee; //keeping tally of total fees. 

					printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%d, fee: $%d.\n"
					,LOG_PREFIX, curr->id, curr->traderID, order->id, order->traderID, value, fee);

					//messaging the relevnat traders that the match has been made.
					char msg1[BUFFER_SIZE];
					sprintf(msg1, "FILL %d %d;", order->id, qty);
					struct trader_struct *trader_temp1 = find_trader_withID(order->traderID);
					if(trader_temp1->ID != -1){
						write_trader(trader_temp1,msg1);
					}

					//messaging the relevnat traders that the match has been made.
					char msg2[BUFFER_SIZE];
					sprintf(msg2, "FILL %d %d;", curr->id, qty);
					struct trader_struct *trader_temp2 = find_trader_withID(curr->traderID);
					if(trader_temp2->ID != -1){
						write_trader(trader_temp2,msg2);
					}

					//adjusting the value of the balances and qtys now that the trade has been finalised. 
					adjust_trader_balance(find_trader_withID(order->traderID),-1 * qty, value - fee, order);
					adjust_trader_balance(find_trader_withID(curr->traderID), qty, -1 * value, order);
					if(curr->qty == 0){
						product->sell_levels -= 1;
						if(curr->next != NULL){
							temp = curr->next;
						}else{
							temp = NULL;
						}
						remove_free_order(product,curr);
						curr = temp;
						freed = TRUE;
						}
					if(order->qty == 0){
						free(order);
						return 0; 
					}
			}else if(strcmp(order->type, "BUY") == 0 && //the order is a 'BUY'
					order->price >= curr->price){ //price needs to be higher than a sell order to qualify as a match
				
				if(order->qty > curr->qty){ //order has a higher quantity than the current node. 
					qty = curr->qty;
					order->qty -= qty;
					curr->qty = 0; 
				}else{ //the order has a lower quantity than the current node.
					qty = order->qty;
					curr->qty -= qty; 
					order->qty = 0;
				}
					value = order->price * qty;
					fee = round(value * 0.01);
					total_fees += fee;

					printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%d, fee: $%d.\n"
					,LOG_PREFIX, curr->id, curr->traderID, order->id, order->traderID, value, fee);

					//messaging the relevnat traders that the match has been made.
					char msg1[BUFFER_SIZE];
					sprintf(msg1, "FILL %d %d;", order->id, qty);
					write_trader(find_trader_withID(order->traderID),msg1);
					
					//messaging the relevnat traders that the match has been made.
					char msg2[BUFFER_SIZE];
					sprintf(msg2, "FILL %d %d;", curr->id, qty);
					struct trader_struct * trader_temp = find_trader_withID(curr->traderID);
					if(trader_temp != NULL){
						if(trader_temp->ID != -1){
							write_trader(trader_temp,msg2);
						}
					}
					//adjusting the value of the balances and qtys now that the trade has been finalised. 
					adjust_trader_balance(find_trader_withID(curr->traderID),-1 * qty, value, order);
					adjust_trader_balance(find_trader_withID(order->traderID),qty, -1 * (value + fee), order); //this is for order
					

					//if the qty of the orders are 0 then they need to be removed from the order linked list. 
					if(curr->qty == 0){
						product->buy_levels -= 1;
						if(curr->next != NULL){
							temp = curr->next;
						}else{
							temp = NULL;
						}
						remove_free_order(product,curr);
						curr = temp;
						freed = TRUE;
						}

					remove_free_order(product,curr);
					if(order->qty == 0){
						free(order);
						return 0; 
					}
				}
			}
		//iteration
		if(freed == TRUE){
			curr = temp;
			freed = FALSE;
		}else{
			curr = curr->next;
		}
		
	}
	return 1;
}


//puts the order into the proper order_struct. 
//prepares and sends a "MARKET" message to all other active traders. 
void process_order(char * order_type, int current_order_id, char * product, int qty, int price, int trader_id){
	order_struct *order = (order_struct *) calloc(1, sizeof(order_struct));
	order->id = current_order_id;
	order->qty = qty;
	order->price = price;
	order->traderID = trader_id;
	strcpy(order->type, order_type);
	strcpy(order->product, product);
	char mass_msg[3 * BUFFER_SIZE];

	//creating the message to send. 
	sprintf(mass_msg,"MARKET %s %s %d %d;",order->type, order->product, order->qty, order->price);
	//identify which product it is in the 'products' malloc. 
	for(int i = 0; i < get_num_products(file_products); i++){
		if(strcmp(order->product,products[i].name) == 0){
			//sends a message to all other traders of the new order that has come.
			write_all_traders(mass_msg, trader_id);
			if(check_matched(&products[i],order) == 0){
				break;
			}
			//add the order into the specific products linked list. 
			add_order(products[i].orders_root, order);
			if(strcmp(order->type, "BUY") == 0){
				products[i].buy_levels += 1;
				break;
			}else if(strcmp(order->type, "SELL") == 0){
				products[i].sell_levels += 1;
				break;
			}
		}
	}
	order_book();
}

//lets the trader know that order message was invalid. 
void invalid_order(struct trader_struct *trader){
	write_trader(trader, "INVALID;");
}

void amend(){

}


void cancel_order(int trader_index, int id){

	
}
