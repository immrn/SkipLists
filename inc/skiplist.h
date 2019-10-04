#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*****************************************************************/
/**************************** Structs ****************************/
/*****************************************************************/

//node
typedef struct _sl_node{
	unsigned int key;
	unsigned int height;
	void* data;
	struct _sl_node* next_in_layer[];
}sl_node;

//skip list
typedef struct{
	sl_node* zero_node;
	unsigned int layer_count;
	unsigned int node_count_in_layer[];
}sl_skip_list;

/*****************************************************************/
/*************************** Functions ***************************/
/*****************************************************************/

/*	This function inserts one node in a skip list at random height and returns true if insertion was successfull.
 *	The function returns false if the insertion failed. It's recommended to remove the same node that should be
 *	inserted and to insert it again.
 *	
 *	PARAMETERS:
 *		-> skiplist: 	- needs a skip list pointer (look at function create_skip_list()).
 *		-> key: 		- Identifies a node, each key can only exist once in a skip list.
 *						- If a node with the same key does already exist it will be replaced
 *						  automatically with the new height. Keys start at 0.
 *						- If a node that's located in front of the first node (zero_node) will be inserted its
 * 						  height is maximum height and the old zero_node is going to be removed and inserted at
 *						  parameter height.
 *		-> data:		- needs a pointer to data.
 *		-> height:		- that's the height of the node, starts at 0.
 */
bool sl_insert_node_static(sl_skip_list* skiplist, unsigned int key, void* data, unsigned int height);

/*	This function inserts one node with random height in a skip list and returns true if insertion was successfull.
 *	The function returns false if the insertion failed. It's recommended to remove the same node that should be
 *	inserted and to insert it again.
 *	
 *	PARAMETERS:
 *		-> skiplist: 	- needs a skip list pointer (look at function create_skip_list()).
 *		-> key: 		- Identifies a node, each key can only exist once in a skip list.
 *						- If a node with the same key does already exist it will be replaced
 *						  automatically with the new height. Keys start at 0.
 *						- If a node that's located in front of the first node (zero_node) will be inserted its
 * 						  height is maximum height and the old zero_node is going to be removed and inserted at
 *						  parameter height.
 *		-> data:		- needs a pointer to data.
 */
bool sl_insert_node(sl_skip_list* skiplist, unsigned int key, void* data);

/*	This function searches through a skip list and returns a node pointer.
 *	The function returns NULL if it wasn't able to find the node.
 *
 *	PARAMETERS:
 *		-> skiplist:	- needs a skip list pointer (look at function create_skip_list())
 *		-> key:			- Function searches for exactly this key. If no node with this key exists in the
 *						  skip list it returns NULL.
 */
sl_node* sl_get_node(sl_skip_list* skiplist, unsigned int key);

/*	This function searches through a skip list and returns a node pointer of the last node in this skip list.
 *	The function returns NULL if the skip list is empty.
 *
 *	PARAMETERS:
 *		-> skiplist:	- needs a skip list pointer (look at function create_skip_list())
 */
sl_node* sl_get_last_node(sl_skip_list* skiplist);

/*	This function removes a node of a skip list and returns true if the node was found and removed.
 *	It also frees the allocated memory of the node.
 *	The function returns false if the node doesn't exist in the skip list.
 *
 *	PARAMETERS:
 *		-> skiplist:	- needs a skip list pointer (look at function create_skip_list())
 *		-> key:			- function searches for exactly this key and deletes the node
 */
bool sl_remove_node(sl_skip_list* skiplist, unsigned int key);

/*	This function removes all nodes of a skip list in a range and returns true if the node was removed successfully.
 *	It also frees the allocated memory of the nodes.
 *	The function returns false if the node doesn't exit in the skip list.
 *
 *	PARAMETERS:
 *		-> skiplist:		- needs a skip list pointer (look at function create_skip_list())
 *		-> minimum_key:		- That's the lowest key that's going to be removed. If this key doesn't exist the
 * 							  functions looks for the next existing node after minimum_key and starts removing
 *							  nodes there until maximum_key.
 *		-> maximum_key:		- thats the highest possible key thats going to be removed
 */
bool sl_remove_node_range(sl_skip_list* skiplist, unsigned int minimum_key, unsigned int maximum_key);


/*	Thisfunction returns a pointer to a skip list whose members are all zeroed/nulled.
 *
 *	WARNING: Everytime using this function check if it returned NULL.
 *			 When it returns NULL there was a error at allocating memory!
 *
 *	PARAMETERS:
 *		-> amount_of_layers:	- height of skip list
 *								- recommended: amount_of_layers = log2(amount of nodes)
 */
sl_skip_list* sl_create_skip_list(unsigned int amount_of_layers);

/*	This function prints the skip list vertically in the console and returns true if it worked correctly.
 *	The function returns false if something went wrong while printing.
 *
 *	PARAMETERS:
 *		-> skiplist:	- needs a skip list pointer (look at function create_skip_list())
 */
bool sl_display_skip_list(sl_skip_list* skiplist);

/*	This function removes all nodes in a skip list.
 *
 *	PARAMETERS:
 *		-> skiplist:	- needs a skip list pointer (look at function create_skip_list())
 */
void sl_remove_skip_list(sl_skip_list* skiplist);