#ifndef _LIST_MODULE_H
#define _LIST_MODULE_H

/*
 * Data structure of a node:
 *	- value is the value of the node
 *	- next is a pointer to the next node
 * The structure is defined under the typename node
 */
typedef struct node {
	int value;
	struct node * next;
} node;

/* Defining the type list as a pointer to a node */
typedef node* list;

/*
 * Assume that the list is in increasing order and insert the element
 * preserving the increasing order.
 * Returns the list with the element inserted.
 */
list list_insert_ordered(list p, int val);

/*
 * Concatenate the last node of before with the next node of the list after
 */
list list_cat(list before, list after);

/*
 * Insert elements at the head of the list
 */
list list_insert_head(list p, int val);

/*
 * Insert elements at the tail of the list
 */
list list_insert_tail(list p, int val);

/*
 * Print the list content
 */
void list_print(list p);

/*
 * Free the list
 */
void list_free(list p);

/*
 * Delete and free the first node of the list head which has the same value
 * of the parameter to_delete.
 * Return the modified list.
 */
list list_delete_if(list head, int to_delete);

/*
 * Delete and free every element in odd position.
 * Return the modified list.
 */
list list_delete_odd(list head);

/*
 * Delete and free every element with value less than the cut_value parameter.
 * Return the modified list.
 */
list list_cut_below(list head, int cut_value);

/*
 * Returns a concrete (copy of every element) copy of the list.
 */
list list_dup(list head);

/*
 * Returns the list without its first element.
 */
list remove_head(list head);

/*
 * Returns the sum of the sizeof of all the list's values.
 */
int list_size(list head);

/*
 * Returns the length of the list (number of nodes != NULL).
 */
int list_length(list head);

/*
 * Converts an int array to a list.
 */
list array_to_list(int* arr, int length);

/*
 * Assuming that l != NULL, returns the last value of the list.
 */
int get_last_element(list l, int prev_val);

/*
 * Returns 1 if l contains val, otherwise returns 0.
 */
char list_contains(list l, int val);

#endif	/*_LIST_MODULE_H*/