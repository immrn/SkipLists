#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "skiplist.h"

/*****************************************************************/
/*********************** (Key) Color Defines *********************/
/*****************************************************************/

#define CNRM  "\x1B[0m"
#define CRED  "\x1B[01;31m"
#define CGRN  "\x1B[01;32m"
#define CYEL  "\x1B[01;33m"
#define CBLU  "\x1B[01;34m"
#define CMAG  "\x1B[01;35m"
#define CCYN  "\x1B[01;36m"
#define CWHT  "\x1B[01;37m"
#define CRESET "\033[0m"

#ifndef KEY_COLOR
#define KEY_COLOR CCYN
#endif /*KEY_COLOR*/

/*****************************************************************/
/************************ Private Functions **********************/
/*****************************************************************/

void increment_node_counts(sl_skip_list* skiplist, unsigned int highest_layer){
	for(int i = 0; i <= highest_layer; i++)
		skiplist->node_count_in_layer[i]++;
}

void decrement_node_counts(sl_skip_list* skiplist, unsigned int highest_layer){
	for(int i = 0; i <= highest_layer; i++)
		skiplist->node_count_in_layer[i]--;
}

int get_digits(unsigned int num){
	long power_of_ten = 10;
	int digits = 1;

	//Count digits of num:
	while(num >= power_of_ten){
		digits++;
		power_of_ten = power_of_ten * 10;
	}
	return digits;
}

unsigned int get_random_int(unsigned int maximum){
	unsigned int rnd = 0;

	//Set seed:
	srand((unsigned int) time(0) - (unsigned int) clock());

	//Generate random integers with geometric distribution X~G(p=0,5),
	//last instance has same probability as forelast instance --> sum of all probabilities = 1:
	while(rnd < maximum && (rand() % 2) == 0)
		rnd++;

	return rnd;
}

char* strcpy_spaces(int count){
	char* str = calloc(1, sizeof(char) * count + 1);

	//Build a string with count spaces:
	for(int i = 0; i < count; i++)
		strcat(str, " ");

	return str;
}

sl_node* create_node(sl_skip_list* skiplist, unsigned int key, void* data){
	sl_node* node = calloc(1, sizeof(sl_node) + sizeof(sl_node*) * skiplist->layer_count);
	//In case calloc() returned NULL, we need to avoid null references:
	if(node == NULL)
		return node;
	node->key = key;
	node->data = data;
	//Shouldn't rely on the machine interpreting that 0ed bits mean NULL:
	for(int i = 0; i < skiplist->layer_count; i++)
		node->next_in_layer[i] = NULL;
	return node;
}

/*****************************************************************/
/************************ Public Functions ***********************/
/*****************************************************************/

bool sl_insert_node_static(sl_skip_list* skiplist, unsigned int key, void* data, unsigned int height){
	//Check whether parameter height is valid:
	if(height > skiplist->layer_count - 1)
		return false;

	//Create the node that shall be inserted:
	sl_node* new_node = create_node(skiplist, key, data);
	//Check whether memory allocation at create_node() worked:
	if(new_node == NULL)
		return false;

	//Case 1: empty skip list, insert the first node
	if (skiplist->zero_node == NULL){
		new_node->height = skiplist->layer_count - 1;
		skiplist->zero_node = new_node;
		increment_node_counts(skiplist, skiplist->layer_count - 1);
	}
	//Case 2: new_node is located behind the zero_node
	else if(new_node->key > skiplist->zero_node->key){
		new_node->height = height;

		//Node pointer that points to the current node in the current layer:
		sl_node* current_node = skiplist->zero_node;
		//Node pointer that points to the next node in the current layer:
		sl_node* next_node;

		//Search and insert layer-wise:
		for(int current_layer = skiplist->layer_count - 1; current_layer >= 0; current_layer--){
			//Get next node in the current layer:
			next_node = current_node->next_in_layer[current_layer];
			
			//Case 2.1: new_node is located behind the last node in current layer
			if(next_node == NULL){
				//Check whether the new node should be inserted in the current layer:
				if(current_layer <= height){
					//Insert new node at current layer: Reset the pointer of current node
					// to point at the new node in the current layer
					current_node->next_in_layer[current_layer] = new_node;
				}
				//Drop down one layer:
				//continue;
			}

			//Case 2.2: new node is located after zero_node and in front of last node
			else if(new_node->key < next_node->key){

				//Check whether the new node should be inserted in the current layer:
				if(current_layer <= height){
					//Insert new node at current layer: Reset the pointer of current node
					// to point at the new node in the current layer:
					current_node->next_in_layer[current_layer] = new_node;
					//Set the pointer of new node to point at the next node in the current layer:
					new_node->next_in_layer[current_layer] = next_node;
				}
				//Drop down one layer:
				//continue;
			}
			else if(new_node->key > next_node->key){
				//Go to the next node in the current layer:
				current_node = current_node->next_in_layer[current_layer];

				//Increment current_layer to stay in current layer because the for-loop decrements current_layer:
				current_layer++;

				//continue;
			}
			//Key of new node does already exist in skip list: overwrite the old node
			else /* new_node->key == next_node->key */{

				//Remove the node with this key because we could have changed some pointers 
				// before we knew that we will "overwrite" this node:
				if(!sl_remove_node(skiplist, key))
					return false;

				//Free new_node after calling sl_remove_node() otherwise the skip list wouldn't be connected correctly:
				free(new_node);

				if(!sl_insert_node_static(skiplist, key, data, height))
					return false;
				
				//Decrement node counts because sl_insert_node_static() did increment it:
				decrement_node_counts(skiplist, height);

				return true;
			}
			
		}
		//new_node was inserted successfully -> increment node_counts of the skip list
		increment_node_counts(skiplist, height);
	}

	//Case 3: new_node is located at zero_node
	else if(new_node->key == skiplist->zero_node->key){
		//Just overwrite the data pointer of zero_node:
		skiplist->zero_node->data = new_node->data;
		//Free new_node because it's dispensable:
		free(new_node);
	}

	//Case 4: new_node is located in front of zero_node
	else /*new_node->key < skiplist->zero_node->key*/{
		//Store key and data pointer of zero_node to insert them later as node:
		int temp_key = skiplist->zero_node->key;
		void* temp_data = skiplist->zero_node->data;

		//new_node gets set as zero_node:
		skiplist->zero_node->key = new_node->key;
		skiplist->zero_node->data = new_node->data;

		//Insert the old zero_node with a random height because there is no given
		//information in which height the old zero_node should be inserted:
		if(!sl_insert_node_static(skiplist, temp_key, temp_data, get_random_int(skiplist->layer_count - 1)))
			return false;

		//Free new_node because it's dispensable:
		free(new_node);

		//increment_node_counts() isn't necessary because sl_insert_node_static() does already increment:
	}
	return true;
}

bool sl_insert_node(sl_skip_list* skiplist, unsigned int key, void* data){
	//Create the node that shall be inserted:
	sl_node* new_node = create_node(skiplist, key, data);
	//Check whether memory allocation in create_node() worked:
	if(new_node == NULL)
		return false;

	//Generate a random height between 0 and (excluded) maximum height + 1:
	int height = get_random_int(skiplist->layer_count - 1);	

	//Case 1: empty skip list, insert the first node
	if (skiplist->zero_node == NULL){
		new_node->height = skiplist->layer_count - 1;
		skiplist->zero_node = new_node;
		increment_node_counts(skiplist, skiplist->layer_count - 1);
	}
	//Case 2: new_node is located behind the zero_node
	else if(new_node->key > skiplist->zero_node->key){
		new_node->height = height;

		//Node pointer that points to the current node in the current layer:
		sl_node* current_node = skiplist->zero_node;
		//Node pointer that points to the next node in the current layer:
		sl_node* next_node;

		//Search and insert layer-wise:
		for(int current_layer = skiplist->layer_count - 1; current_layer >= 0; current_layer--){
			//Get next node in current layer:
			next_node = current_node->next_in_layer[current_layer];
			
			//Case 2.1: new_node is located behind the last node in current layer
			if(next_node == NULL){
				//Check whether the new node should be inserted in the current layer:
				if(current_layer <= height){
					//Insert new node at current layer: Reset the pointer of current node
					// to point at the new node in the current layer:
					current_node->next_in_layer[current_layer] = new_node;
				}
				//Drop one layer down:
				//continue;
			}

			//Case 2.2: new node is located after zero_node and in front of last node
			else if(new_node->key < next_node->key){

				//Check whether the new node should be inserted in the current layer:
				if(current_layer <= height){
					//Insert new node at current layer: Reset the pointer of current node
					// to point at the new node in the current layer:
					current_node->next_in_layer[current_layer] = new_node;
					//Set the pointer of new node to point at the next node in the current layer:
					new_node->next_in_layer[current_layer] = next_node;
				}
				//Drop one layer down:
				//continue;
			}
			else if(new_node->key > next_node->key){
				//Go to the next node in the current layer:
				current_node = current_node->next_in_layer[current_layer];

				//Increment current_layer to stay in current layer because the for-loop decrements current_layer:
				current_layer++;

				//continue;
			}
			//Key of new node does already exist in skip list:
			else /* new_node->key == next_node->key */{

				//Remove the node with this key because we could have changed some pointers 
				// before we knew that we will "overwrite" this node:
				if(!sl_remove_node(skiplist, key))
					return false;

				//Free new_node after calling sl_remove_node() otherwise the skip list wouldn't be connected correctly:
				free(new_node);

				if(!sl_insert_node_static(skiplist, key, data, height))
					return false;

				//Decrement node counts because sl_insert_node_static did increment it:
				decrement_node_counts(skiplist, height);

				return true;
			}
			
		}
		//new_node was inserted successfully -> increment node_counts of the skip list
		increment_node_counts(skiplist, height);
	}

	//Case 3: new_node is located at zero_node
	else if(new_node->key == skiplist->zero_node->key){
		//Overwrite the data pointer of zero_node:
		skiplist->zero_node->data = new_node->data;
		//Free new_node because it's dispensable:
		free(new_node);
	}

	//Case 4: new_node is located before zero_node
	else /* new_node->key < skiplist->zero_node->key */{
		//Store key and data pointer of zero_node to insert them later as node:
		int temp_key = skiplist->zero_node->key;
		void* temp_data = skiplist->zero_node->data;

		//new_node gets set as zero_node:
		skiplist->zero_node->key = new_node->key;
		skiplist->zero_node->data = new_node->data;

		//Insert the old zero_node with the height that was calculated on beginning of this function:
		if(!sl_insert_node_static(skiplist, temp_key, temp_data, height))
			return false;

		//Free new_node because it's dispensable:
		free(new_node);

		//increment_node_counts() isn't necessary because sl_insert_node_static() does already increment:
	}
	return true;
}

sl_node* sl_get_node(sl_skip_list* skiplist, unsigned int key){
	//Check whether skip list is empty or key is located in front of zero_node:
	if(skiplist->zero_node == NULL  ||  key < skiplist->zero_node->key){
		return NULL;
	}
	//Search for the wanted node:
	else{
		//Node pointer that points to the current node in the current layer:
		sl_node* current_node = skiplist->zero_node;

		//Search layer-wise, start at highest layer:
		for(int current_layer = skiplist->layer_count - 1; current_layer >= 0; current_layer--){
			//Check whether wanted node was found:
			if(key == current_node->key){
				return current_node;
			}
			//Check whether wanted node isn't located in current layer
			else if(current_node->next_in_layer[current_layer] == NULL ||
					current_node->next_in_layer[current_layer]->key > key){
				//Drop one layer down:
				//continue;
			}
			else /*(key >= current_node->next_in_layer[current_layer]->key)*/{
				//Go to the next node in the current layer:
				current_node = current_node->next_in_layer[current_layer];
				//Increment current_layer to stay in current layer because the for-loop decrements current_layer:
				current_layer++;
				//continue;
			}	
		}
		return NULL;
	}
}

sl_node* sl_get_last_node(sl_skip_list* skiplist){
	//Check whether skip list is empty:
	if(skiplist->zero_node == NULL){
		return NULL;
	}

	//Node pointer that points to the current node in the current layer:
	sl_node* current_node = skiplist->zero_node;

	//Search layer-wise, start at highest layer:
	for(int current_layer = skiplist->layer_count - 1; current_layer >= 0; current_layer--){
		if(current_node->next_in_layer[current_layer] == NULL){
			//Drop down one layer:
			//continue;
		}
		else{
			//Stay in current layer and go to next node:
			current_node = current_node->next_in_layer[current_layer];
			current_layer++;
			//continue;
		}
	}
	//Reached layer 0 and got the last node of the skip list, return it:
	return current_node;
}

bool sl_remove_node(sl_skip_list* skiplist, unsigned int key){
	//When this function found the node that is going to be removed than it's necessary
	//to store it in remove_node once to free its allocated memory later.
	//To remember if this was done already we need a bool:
	bool is_remove_node_selected = false;
	sl_node* remove_node;

	//Check whether skip list is empty or remove_node is located in front of zero_node:
	if(skiplist->zero_node == NULL  ||  key < skiplist->zero_node->key)
		return false;

	//Check whether zero_node must be removed:
	if(key == skiplist->zero_node->key){
		//Store the address of the node that's going to be removed to free its memory later:
		remove_node = skiplist->zero_node;

		//Create new_zero_node:
		sl_node* new_zero_node = skiplist->zero_node->next_in_layer[0];

		//If skip list contains only one node:
		if(new_zero_node == NULL){
			decrement_node_counts(skiplist, remove_node->height);
			//empty the skip list:
			skiplist->zero_node = NULL;
			free(remove_node);
			return true;
		}

		//Transfer the next_in_layer pointers of zero_node to the next_in_layer pointers of
		//new_zero_node in the corresponding layers but only those which are higher than height of new_zero_node:
		for(int i = new_zero_node->height + 1; i < skiplist->layer_count; i++){
			new_zero_node->next_in_layer[i] = skiplist->zero_node->next_in_layer[i];
		}

		decrement_node_counts(skiplist, remove_node->height);

		//Set new_zero_node as zero_node of the skip list:
		new_zero_node->height = skiplist->layer_count - 1;
		skiplist->zero_node = new_zero_node;
		free(remove_node);
		return true;
	}
	//The node that's going to be removed is located behind the zero node:
	else{
		//Node pointer that points to the current node in the current layer:
		sl_node* current_node = skiplist->zero_node;
		//Node pointer that points to the next node in the current layer:
		sl_node* next_node;

		//Search and reset pointers (next_in_layer) layer-wise:
		for(int current_layer = skiplist->layer_count - 1; current_layer >= 0; current_layer--){
			//Get next node in layer:
			next_node = current_node->next_in_layer[current_layer];
			
			//Check wether current layer is empty behind current_node or key < key of next node:
			if(next_node == NULL || key < next_node->key){
				//Check whether the current layer == 0
				if(current_layer == 0){
					//Stop removing because the key isn't in this skip list:
					return false;
				}
				//Drop down one layer:
				//continue;
			}
			else if(key > next_node->key){
				//Go to the next node in the current layer:
				current_node = current_node->next_in_layer[current_layer];
				//Increment current_layer to stay in current layer because the for-loop decrements current_layer:
				current_layer++;
				//continue;
			}
			else /* key == next_node->key */{
				//Found the pointer (current_node->next_in_layer[current_layer]) that points at the node wich is going to be removed:
				if(!is_remove_node_selected){
					//Store the node that's going to be removed to free its memory later:
					remove_node = current_node->next_in_layer[current_layer];
					is_remove_node_selected = true;
				}

				//current_node points now at the node in current layer that comes after the node that is going to be removed:
				current_node->next_in_layer[current_layer] = next_node->next_in_layer[current_layer];

				if(current_layer == 0){
					decrement_node_counts(skiplist, remove_node->height);
					//Free the allocated memory:
					free(remove_node);
					break;
				}
				//Drop down one layer:
				//continue;
			}	
		}
		//Node was removed:
		return true;
	}
}

bool sl_remove_node_range(sl_skip_list* skiplist, unsigned int minimum_key, unsigned int maximum_key){
	//Check validity of parameters:
	if(minimum_key >= maximum_key)
		return false;

	//Check whether skip list is empty:
	else if(skiplist->zero_node == NULL)
		return false;

	//Remove nodes in given range:
	else{
		sl_node* current_node = sl_get_node(skiplist, minimum_key);

		//If no node with minimum_key does exist in skip list search for next greater key:
		while(current_node == NULL)
			current_node = sl_get_node(skiplist, minimum_key++);

		//Remove nodes in given range
		while(current_node != NULL  &&  current_node->key <= maximum_key){
			//Check sl_remove_node() because it returns false if no node was deleted.
			//Because it's garanteed (here) that everytime we call sl_remove_node() it should return true,
			//it's necessary to return false when one single call of sl_remove_node() returns false:
			if(!sl_remove_node(skiplist, current_node->key))
				return false;
			//Get next node:
			current_node = current_node->next_in_layer[0];
		}

		//All nodes were removed successfully:
		return true;
	}
}

sl_skip_list* sl_create_skip_list(unsigned int layers){
	//Check parameter:
	if(layers == 0)
		return NULL;

	//Allocate memory, set all bytes to 0:
	sl_skip_list* skiplist = calloc(1, sizeof(sl_skip_list) + sizeof(int) * layers);
	//In case calloc() returned NULL, we need to avoid null references:
	if(skiplist == NULL)
		return skiplist;
	//Set layer_count:
	skiplist->layer_count = layers;
	//Shouldn't rely on the machine interpreting that 0ed bits mean NULL:
	skiplist->zero_node = NULL;

	return skiplist;
}

bool sl_display_skip_list(sl_skip_list* skiplist){
	//Check whether the skip list is empty:
	if(skiplist->zero_node == NULL)
		return true;

	//Get information about how many spaces are needed for a good format:
	unsigned int last_Key = sl_get_last_node(skiplist)->key;
	int max_digits = get_digits(last_Key);

	//Allocate memory: spacer_1 for spaces that will be printed before the key is printed,
	//spacer_2 for spaces that will be printed before "|" wich signals that the pointer of
	//the node with the key above "|" points to the node with the key underneath "|"
	char* spacer_1 = calloc(1, sizeof(char) * max_digits + 1);
	char* spacer_2 = calloc(1, sizeof(char) * (max_digits - 1) + 1);

	//Check if calloc() returned NULL:
	if(spacer_1 == NULL  ||  spacer_2 == NULL){
		//Free spacer:
		free(spacer_1);
		free(spacer_2);
		return false;
	}

	//Free spacer_2 because strcpy_spaces is calling calloc() again and returning a pointer:
	free(spacer_2);
	spacer_2 = strcpy_spaces(max_digits - 1);

	//Check if strcpy_spaces returned NULL:
	if(spacer_2 == NULL){
		//Free spacer:
		free(spacer_1);
		free(spacer_2);
		return false;
	}

	//When printing a line/node it's necessary to know wich layer has still a open connection:
	int last_height_with_opened_connection = skiplist->layer_count - 1;

	sl_node* current_node = skiplist->zero_node;

	/*** Go through layer 0 and print every node in one line ***/
	//while-loop breaks when last node was printed then current_node == NULL:
	while(current_node != NULL){
		//Update last_height_with_opened_connection:
		if(current_node->height == last_height_with_opened_connection){
			while(current_node->next_in_layer[last_height_with_opened_connection] == NULL)
				last_height_with_opened_connection--;
		}

		//Free spacer_1 because strcpy_spaces is calling calloc() again and returning a pointer:
		free(spacer_1);
		
		//Calculate how many spaces must be added to the string of current key to have
		//the same string length like the highest key of skip list:
		
		spacer_1 = strcpy_spaces(max_digits - get_digits(current_node->key));

		//Check if strcpy_spaces() returned NULL:
		if(spacer_1 == NULL){
			//Free spacer:
			free(spacer_1);
			free(spacer_2);
			return false;
		}

		//Print the strings with spacer_1 and current key n times, n = height of current node:
		for(int i = 0; i <= current_node->height; i++)
			printf("%s" KEY_COLOR "%d   " CRESET, spacer_1, current_node->key);
		
		//Print the strings with spacer_2 and "|" m times, m = (height of current node) - (last_height_with_opened_connection):
		for(int i = current_node->height; i < last_height_with_opened_connection; i++)
			printf("%s|   ", spacer_2);

		//Go into next line to print a line that only contains spaces and "|":
		printf("\n");

		//Break loop when we already printed the last node:
		if(current_node->next_in_layer[0] == NULL)
			break;

		//Print the strings with spacer_2 and "|" k times, k = last_height_with_opened_connection + 1:
		for(int i = 0; i <= last_height_with_opened_connection; i++)
			printf("%s|   ", spacer_2);

		//Go into next line:
		printf("\n");

		//Go to next node in layer 0:
		current_node = current_node->next_in_layer[0];
	}
	//Free spacer:
	free(spacer_1);
	free(spacer_2);

	return true;
}	

void sl_remove_skip_list(sl_skip_list* skiplist){
	sl_node* current_node = skiplist->zero_node;
	sl_node* next_node;

	//Remove all nodes:
	while(current_node != NULL){
		next_node = current_node->next_in_layer[0];
		free(current_node);
		current_node = next_node;
	}

	//Free allocated memory of the skip list:
	free(skiplist);
	return;
}