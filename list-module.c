#include <stdio.h>
#include <stdlib.h>
#include "list-module.h"

list list_insert_ordered(list p, int val)
{
	list i, l;
	if(p->value > val)
		return list_insert_head(p, val);
	i = p;
	while(i->next != NULL && i->next->value <= val)
		i = i->next;
	l = malloc(sizeof(*l));
	l->value = val;
	if(i->next == NULL)
		l->next = NULL;
	else
		l->next = i->next;
	i->next = l;
	return p;
}

list list_cat(list before, list after)
{
	list p = before;
	if(p == NULL)
		return after;
	else if(p == NULL)
		return before;
	while(p->next != NULL)
		p = p->next;
	p->next = after;
	return before;
}

list list_insert_tail(list p, int val)
{
	list i, l = NULL;
	l = malloc(sizeof(*l));
	l->value = val;
	l->next = NULL;
	if(p == NULL)
		return l;
	i = p;
	while(i->next != NULL)
		i = i->next;
	i->next = l;
	return p;
}

list list_insert_head(list p, int val)
{
	int a;
	list new_el = NULL;

	/* Allocate the new node */
	a = sizeof(*new_el);
	/*new_el = malloc(sizeof(*new_el));*/
	new_el = malloc(a);
	/* Setting the data */
	new_el->value = val;
	/* head insertion: old head becomes .next field of new head */
	new_el->next = p;
	/* new head is the pointer to the new node */
	return new_el;
}

void list_print(list p)
{
	/* Looping all nodes until p == NULL */
	if (p == NULL) {
		printf("Empty list\n");
		return;
	}
	printf("[%i]", p->value);
	for(; p->next!=NULL; p = p->next) {
		printf(" -> [%i]", p->next->value);
	}
	printf("\n");
}

void list_free(list p)
{
	/* If p ==  NULL, nothing to deallocate */
	if (p == NULL) {
		return;
	}
	/* First deallocate (recursively) the next nodes... */
	list_free(p->next);
	/* ... then deallocate the node itself */
	free(p);
}

list list_delete_if(list head, int to_delete)
{
	list l;
	if(head == NULL)
		return NULL;
	else if(head->value == to_delete)
	{
		list l = head->next;
		free(head);
		return l;
	}
	l = head;
	while(l->next != NULL)
	{
		if(l->next->value == to_delete)
		{
			list n = l->next;
			l->next = l->next->next;
			free(n);
			return head;
		}
		l = l->next;
	}
	return head;
}

list list_delete_odd(list head)
{
	list l = head;
	while(l != NULL && l->next != NULL)
	{
		list to_free = l->next;
		l->next = l->next->next;
		free(to_free);
		l = l->next;
	}
	return head;
}

list list_cut_below(list head, int cut_value)
{
	list l;
	while(head != NULL && head->value < cut_value)
	{
		list to_free = head;
		head = head->next;
		free(to_free);
	}
	l = head;
	while(l != NULL && l->next != NULL)
	{
		if(l->next->value < cut_value)
		{
			list to_free = l->next;
			l->next = l->next->next;
			free(to_free);
		}
		else
			l = l->next;
	}
	return head;
}

list list_dup(list head)
{
	list l, i;
	l = malloc(sizeof(*l));
	i = l;
	while(i != NULL)
	{
		i->value = head->value;
		if(head->next != NULL)
			i->next = malloc(sizeof(*i));
		else
			i->next = NULL;
		i = i->next;
		head = head->next;
	}
	return l;
}

list remove_head(list head)
{
	list l;
	if(head == NULL) return NULL;
	l = head->next;
	head->next = NULL;
	free(head);
	return l;
}

int list_size(list head)
{
	if(head == NULL)
	{
		return 0;
	}
	return sizeof(head->value) + list_size(head->next);
}

int list_length(list head)
{
	if(head == NULL)
		return 0;
	return 1 + list_length(head->next);
}

list array_to_list(int* arr, int length)
{
	list l = NULL;
	int i;
	for(i = 0; i < length; i++)
	{
		l = list_insert_tail(l, arr[i]);
	}
	return l;
}

int get_last_element(list l, int prev_val)
{
	if(l == NULL)
		return prev_val;
	else
		return get_last_element(l->next, l->value);
}

char list_contains(list l, int val)
{
	while(l != NULL)
	{
		if(l->value == val) return 1;
		l = l->next;
	}
	return 0;
}