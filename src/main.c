#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "skiplist.h"

//Needed by example:
#define LAYERS 4
#define NODES 15

//Data for the example:
typedef struct{
	char* name;
	char* achievement;
	int year;
}Developer;

//Example application:
int Example();

//Benchmarks: Call one of the
void Benchmark01();
void Benchmark02();
void Benchmark03();

//Functions used by Benchmarks:
bool benchmark_insert_search_remove(int layers, unsigned int nodes, int iterations, float factor);
bool benchmark_build_sl(int layers, unsigned nodes, int iterations);

int main(void){
	Example();
	return 0;
}

int Example(){	
	//Create a skip list:
	sl_skip_list *skp = sl_create_skip_list(LAYERS);

	//Insert some nodes:
	for(int i = 0; i < NODES; i++){
		if(!sl_insert_node(skp, i, NULL)){
			printf("Error while inserting a node!\n");
			return -1;
		}
	}

	//Now print the skip list vertically:
	if(!sl_display_skip_list(skp)){
		printf("\nSL was'nt printed correctly\n");
	}

	//We want to remove the last node:
	printf("\n\nRemoving last node...\n");
	if(!sl_remove_node(skp, sl_get_last_node(skp)->key)){
		printf("Error while removing a node!\n");
		return -3;
	}

	//Print skip list again:
	if(!sl_display_skip_list(skp)){
		printf("\nSL was'nt printed correctly\n");
	}

	//Generate some data for data pointer:
	Developer william = { 	.name = "William Pugh",
							.achievement = "skip lists",
							.year = 1989 };

	//Insert a node with this data:
	if(!sl_insert_node(skp, 12, &william)){
		printf("Error while inserting a node!\n");
		return -4;
	}

	//Get the node with this data:
	sl_node *dataNode = sl_get_node(skp, 12);

	//Check whether sl_get_node() did return NULL:
	if(dataNode == NULL){
		printf("Couldn't find node!\n");
		return -5;
	}

	//safe data:
	Developer *new_Dev = dataNode->data;

	//print the data:
	printf("\nTell me something about William Pugh:\n");
	printf("%s developed %s in %d!\n", new_Dev->name, new_Dev->achievement, new_Dev->year);

	//remove some of the last nodes:
	if(!sl_remove_node_range(skp, 5, 15)){
		printf("Error while removing nodes in a row!\n");
		return -6;
	}

	//print skip list again:
	if(!sl_display_skip_list(skp)){
		printf("\nSL was'nt printed correctly\n");
	}

	//remove skip list:
	sl_remove_skip_list(skp);
	
	return 0;
}

void Benchmark01(){
	//Compare insert/search/remove time of singly linked list and skip list:
	
	printf("\n--- Compare time for insertion/search/remove of singly linked list and skip list\n\n");

	//Singly linked list:
	printf("Singly Linked List:\n");
	benchmark_insert_search_remove(1, 10000, 1000, 0.8);
	printf("\n\n\n");

	//Skip list:
	printf("Skip List:\n");
	benchmark_insert_search_remove(13, 10000, 10000, 0.8);
}

void Benchmark02(){
	//Compare build time of 4 skip lists with a different amount of layers:

	printf("\n--- Compare time for building up skiplists with a different amount of layers\n\n");

	//Skip list 0 with 10 layers (not recommended):
	printf("Skip List 0:\n");
	benchmark_build_sl(10, 10000, 1000);
	printf("\n\n");

	//Skip list 1 with log2(10000) approx 13 layers (recommended):
	printf("Skip List 1:\n");
	benchmark_build_sl(13, 10000, 1000);
	printf("\n\n");

	//Skip list 2 with 30 layers (not recommended):
	printf("Skip List 2:\n");
	benchmark_build_sl(30, 10000, 1000);
	printf("\n\n");

	//Skip list 3 with 100 layers (not recommended):
	printf("Skip List 3:\n");
	benchmark_build_sl(100, 10000, 1000);
}

void Benchmark03(){
	//Compare insert/search/remove time of three skiplists with a different amount of nodes:
	
	printf("--- Compare three skiplists with a different amount of nodes\n\n");

	//Skip list 1:
	printf("Skip List 1:\n");
	benchmark_insert_search_remove(13, 10000, 1000, 0.8);
	printf("\n\n");

	//Skip list 2:
	printf("Skip List 2:\n");
	benchmark_insert_search_remove(13, 100000, 1000, 0.8);
	printf("\n\n");	

	//Skip list 3:
	printf("Skip List 3:\n");
	benchmark_insert_search_remove(13, 1000000, 100, 0.8);
}

bool benchmark_insert_search_remove(int layers, unsigned int nodes, int iterations, float factor){

	double summed_insertion_time = 0;
	double summed_searching_time = 0;
	double summed_removing_time = 0;
	unsigned summed_unused_layers = 0;
	
	//Duplicate nodes because the skiplist gets only filled with odd keys:
	nodes *= 2;

	//Must be even:
	unsigned int wanted_key = factor * nodes;

	//Iteration loop:
	for(int i = 0; i < iterations; i++){

		//Create skiplist:
		sl_skip_list *skp = sl_create_skip_list(layers);

		//Insertion of nodes with odd keys:
		for(int j = 1; j <= nodes; j += 2){
			if(!sl_insert_node(skp, j, NULL)){
				printf("Error while building up the whole skiplist\n");
				return false;
			}
		}

		//Information about which layers are unused:
		int curr_layer = layers - 1;
		while(skp->zero_node->next_in_layer[curr_layer] == NULL){
			summed_unused_layers++;
			curr_layer--;
		}

		//Benchmarking of one insertion:
		clock_t start = clock();
		sl_insert_node(skp,wanted_key, NULL);
		summed_insertion_time += (double)(clock() - start) / (CLOCKS_PER_SEC / 1000000);

		//Benchmarking of one search:
		start = clock();
		sl_node *even_node = sl_get_node(skp, wanted_key);
		summed_searching_time += (double)(clock() -start) / (CLOCKS_PER_SEC / 1000000);

		//Check if insertion and search were successfull:
		if(even_node == NULL || even_node->key != wanted_key){
			printf("Error while inserting/searching the node with wanted key into skiplist\n");
			return false;
		}

		//Benchmarking of removing one node:
		start = clock();
		sl_remove_node(skp, wanted_key);
		summed_removing_time += (double)(clock() -start) / (CLOCKS_PER_SEC / 1000000);

		//Check if removing was successfull:
		even_node = sl_get_node(skp, wanted_key);
		if(even_node != NULL){
			printf("Error while removing the node with wanted key\n");
			return false;
		}

		//Remove skiplist:
		sl_remove_skip_list(skp);
	}

	//Calculate averages:
	double average_unused_layers = (double)summed_unused_layers / (double)iterations;
	double average_insertion_time = summed_insertion_time / (double)iterations;
	double average_searching_time = summed_searching_time / (double)iterations;
	double average_removing_time = summed_removing_time / (double)iterations;

	printf("\twanted key:\t\t\t%d\n", wanted_key);
	printf("\titerations:\t\t\t%d\n", iterations);
	printf("\tnodes:\t\t\t\t%d\n", nodes/2);
	printf("\tlayers:\t\t\t\t%d\n", layers);
	printf("\taverage unused layers:\t\t%lf\n", average_unused_layers);
	printf("\n");
	printf("\taverage time for insertion:\t%.2lf μs\n", average_insertion_time);
	printf("\taverage time for searching:\t%.2lf μs\n", average_searching_time);
	printf("\taverage time for removing:\t%.2lf μs\n", average_removing_time);

	return true;
}

bool benchmark_build_sl(int layers, unsigned nodes, int iterations){
	double summed_build_time = 0;
	unsigned summed_unused_layers = 0;

	for(int i = 0; i < iterations; i++){
		clock_t start = clock();
		//Create skiplist:
		sl_skip_list *skp = sl_create_skip_list(layers);

		//Insertion of nodes:
		for(int j = 1; j <= nodes; j++){
			if(!sl_insert_node(skp, j, NULL)){
				printf("Error while building up the whole skiplist\n");
				return false;
			}
		}

		summed_build_time += (double)(clock() - start) / (CLOCKS_PER_SEC / 1000000);

		//Information about which layers are unused:
		int curr_layer = layers - 1;
		while(skp->zero_node->next_in_layer[curr_layer] == NULL){
			summed_unused_layers++;
			curr_layer--;
		}

		sl_remove_skip_list(skp);
	}

	double average_build_time = summed_build_time / (double)iterations;
	double average_unused_layers = (double)summed_unused_layers / (double)iterations;

	printf("\titerations:\t\t\t%d\n", iterations);
	printf("\tnodes:\t\t\t\t%d\n", nodes);
	printf("\tlayers:\t\t\t\t%d\n", layers);
	printf("\taverage unused layers:\t\t%lf\n", average_unused_layers);
	printf("\taverage time for building:\t%.2lf μs\n", average_build_time);

	return true;
}
