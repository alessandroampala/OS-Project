#include <stdlib.h>
#include "pathfinding.h"
#include "list-module.h"
#include "player-module.h"

int index_pedina(int pos_pedina, pedina* pedine, int num_pedine)
{
	int i;
	for (i = 0; i < num_pedine; ++i)
	{
		if(pedine[i].position == pos_pedina)
			return i;
	}
	return -1;
}

int pedina_assegnata(int pos_pedina, pedina* pedine, int num_pedine)
{
	if(pedine[index_pedina(pos_pedina, pedine, num_pedine)].bandierina_assegnata != -1)
		return 1;
	else
		return 0;
}

list bfs(int start, int goal, char* scacchiera, int base, int altezza)
{
	int i, dim, current;
	path_node* graph;
	list q;
	list l = NULL;

	dim = base*altezza;

	graph = malloc(dim * sizeof(path_node));
	for(i = 0; i < dim; i++)
	{
		if(scacchiera[i] != '-' && scacchiera[i] != '>')
			graph[i].is_visited = 1;
		else
			graph[i].is_visited = 0;
		graph[i].parent = -1;
	}

	q = NULL;
	q = list_insert_head(q, start);
	graph[start].is_visited = 1;
	graph[start].parent = -10;

	while(q != NULL)
	{
		current = q->value;
		q = remove_head(q);

		/*figlio sopra*/
		if(current - base >= 0 && graph[current - base].is_visited == 0)
		{
			graph[current - base].is_visited = 1;
			graph[current - base].parent = current;
			if(current - base == goal)
			{
				list to_return;
				current = goal;
				l = list_insert_head(l, goal);
				/*return path*/
				while(graph[current].parent != -10)
				{
					l = list_insert_head(l, graph[current].parent);
					current = graph[current].parent;
				}
				to_return = l->next;
				free(graph);
				free(l);
				list_free(q);
				return to_return;
			}
			q = list_insert_tail(q, current - base);
		}

		/*figlio sotto*/
		if(current + base < dim && graph[current + base].is_visited == 0)
		{
			graph[current + base].is_visited = 1;
			graph[current + base].parent = current;
			if(current + base == goal)
			{
				list to_return;
				current = goal;
				l = list_insert_head(l, goal);
				/*return path*/
				while(graph[current].parent != -10)
				{
					l = list_insert_head(l, graph[current].parent);
					current = graph[current].parent;
				}
				to_return = l->next;
				free(graph);
				free(l);
				list_free(q);
				return to_return;
			}
			q = list_insert_tail(q, current + base);
		}

		/*figlio destra*/
		if(current + 1 < dim && ((current + 1) % base > current % base) && graph[current + 1].is_visited == 0)
		{
			graph[current + 1].is_visited = 1;
			graph[current + 1].parent = current;
			if(current + 1 == goal)
			{
				list to_return;
				current = goal;
				l = list_insert_head(l, goal);
				/*return path*/
				while(graph[current].parent != -10)
				{
					l = list_insert_head(l, graph[current].parent);
					current = graph[current].parent;
				}
				to_return = l->next;
				free(graph);
				free(l);
				list_free(q);
				return to_return;
			}
			q = list_insert_tail(q, current + 1);
		}

		/*figlio sinistra*/
		if(current - 1 >= 0 && ((current - 1) % base < current % base) && graph[current - 1].is_visited == 0)
		{
			graph[current - 1].is_visited = 1;
			graph[current - 1].parent = current;
			if(current - 1 == goal)
			{
				list to_return;
				current = goal;
				l = list_insert_head(l, goal);
				/*return path*/
				while(graph[current].parent != -10)
				{
					l = list_insert_head(l, graph[current].parent);
					current = graph[current].parent;
				}
				to_return = l->next;
				free(graph);
				free(l);
				list_free(q);
				return to_return;
			}
			q = list_insert_tail(q, current - 1);
		}
	}
	free(graph);
	list_free(l);
	list_free(q);
	return NULL;
}

nearest_paw find_nearest_paw(int start_flag, char* scacchiera, int base, int altezza,
						pedina* pedine, int num_pedine, char identifier)
{
	int i, dim, current;
	path_node* graph;
	list q;
	nearest_paw ret;
	list l = NULL;
	char ostacolo = 0;

	dim = base*altezza;

	graph = malloc(dim * sizeof(path_node));
	for(i = 0; i < dim; i++)
	{
		if(scacchiera[i] != '-' && scacchiera[i] != identifier && scacchiera[i] != '>')
			graph[i].is_visited = 1;
		else
			graph[i].is_visited = 0;
		graph[i].parent = -1;
	}

	q = NULL;
	q = list_insert_head(q, start_flag);
	graph[start_flag].is_visited = 1;
	graph[start_flag].parent = -10;
	ret.index = -1;
	ret.path = NULL;

	while(q != NULL)
	{
		current = q->value;
		q = remove_head(q);

		/*figlio sopra*/
		if(current - base >= 0 && graph[current - base].is_visited == 0)
		{
			graph[current - base].is_visited = 1;
			graph[current - base].parent = current;
			if(scacchiera[current - base] == identifier && !pedina_assegnata(current - base, pedine, num_pedine))
			{
				int mosse = 0;
				int target = current - base;
				/*return path*/
				while(graph[target].parent != -10)
				{
					l = list_insert_tail(l, graph[target].parent);
					target = graph[target].parent;
					mosse++;
				}

				if(mosse <= pedine[index_pedina(current - base, pedine, num_pedine)].mosse_residue)
				{
					list_free(q);
					free(graph);
					ret.path = l;
					ret.index = index_pedina(current - base, pedine, num_pedine);
					return ret;
				}
				else
				{
					ostacolo = 1;
					list_free(l);
					l = NULL;
				}
			}

			if(ostacolo)
			{
				ostacolo = 0;
			}
			else
			{
				q = list_insert_tail(q, current - base);
			}
		}

		/*figlio sotto*/
		if(current + base < dim && graph[current + base].is_visited == 0)
		{
			graph[current + base].is_visited = 1;
			graph[current + base].parent = current;
			if(scacchiera[current + base] == identifier && !pedina_assegnata(current + base, pedine, num_pedine))
			{
				int mosse = 0;
				int target = current + base;
				/*current = current + base;*/
				/*return path*/
				while(graph[target].parent != -10)
				{
					l = list_insert_tail(l, graph[target].parent);
					target = graph[target].parent;
					mosse++;
				}

				if(mosse <= pedine[index_pedina(current + base, pedine, num_pedine)].mosse_residue)
				{
					list_free(q);
					free(graph);
					ret.path = l;
					ret.index = index_pedina(current + base, pedine, num_pedine);
					return ret;
				}
				else
				{
					ostacolo = 1;
					list_free(l);
					l = NULL;
				}
			}

			if(ostacolo)
			{
				ostacolo = 0;
			}
			else
			{
				q = list_insert_tail(q, current + base);
			}
		}

		/*figlio destra*/
		if(current + 1 < dim && ((current + 1) % base > current % base) && graph[current + 1].is_visited == 0)
		{
			graph[current + 1].is_visited = 1;
			graph[current + 1].parent = current;
			if(scacchiera[current + 1] == identifier && !pedina_assegnata(current + 1, pedine, num_pedine))
			{
				int mosse = 0;
				int target = current + 1;
				/*return path*/
				while(graph[target].parent != -10)
				{
					l = list_insert_tail(l, graph[target].parent);
					target = graph[target].parent;
					mosse++;
				}

				if(mosse <= pedine[index_pedina(current + 1, pedine, num_pedine)].mosse_residue)
				{
					list_free(q);
					free(graph);
					ret.path = l;
					ret.index = index_pedina(current + 1, pedine, num_pedine);
					return ret;
				}
				else
				{
					ostacolo = 1;
					list_free(l);
					l = NULL;
				}
			}

			if(ostacolo)
			{
				ostacolo = 0;
			}
			else
			{
				q = list_insert_tail(q, current + 1);
			}
		}

		/*figlio sinistra*/
		if(current - 1 >= 0 && ((current - 1) % base < current % base) && graph[current - 1].is_visited == 0)
		{
			graph[current - 1].is_visited = 1;
			graph[current - 1].parent = current;
			if(scacchiera[current - 1] == identifier && !pedina_assegnata(current - 1, pedine, num_pedine))
			{
				int mosse = 0;
				int target = current - 1;
				/*return path*/
				while(graph[target].parent != -10)
				{
					l = list_insert_tail(l, graph[target].parent);
					target = graph[target].parent;
					mosse++;
				}

				if(mosse <= pedine[index_pedina(current - 1, pedine, num_pedine)].mosse_residue)
				{
					list_free(q);
					free(graph);
					ret.path = l;
					ret.index = index_pedina(current - 1, pedine, num_pedine);
					return ret;
				}
				else
				{
					ostacolo = 1;
					list_free(l);
					l = NULL;
				}
			}

			if(ostacolo)
			{
				ostacolo = 0;
			}
			else
			{
				q = list_insert_tail(q, current - 1);
			}
		}
	}
	list_free(q);
	free(graph);
	list_free(l);
	ret.path = NULL;
	return ret;
}

list find_nearest_flag(int start, char* scacchiera, int base, int altezza, int mosse_residue, char* assigned_flag)
{
	int i, dim, current;
	path_node* graph;
	list q;
	list l = NULL;

	dim = base*altezza;

	graph = malloc(dim * sizeof(path_node));
	for(i = 0; i < dim; i++)
	{
		if(scacchiera[i] != '-' && scacchiera[i] != '>')
			graph[i].is_visited = 1;
		else
			graph[i].is_visited = 0;
		graph[i].parent = -1;
	}

	q = NULL;
	q = list_insert_head(q, start);
	graph[start].is_visited = 1;
	graph[start].parent = -10;

	while(q != NULL)
	{
		current = q->value;
		q = remove_head(q);

		/*figlio sopra*/
		if(current - base >= 0 && graph[current - base].is_visited == 0)
		{
			graph[current - base].is_visited = 1;
			graph[current - base].parent = current;
			if(scacchiera[current - base] == '>' && assigned_flag[current - base] == 0)
			{

				int mosse = 0;
				int end_node = current - base;
				int target = current - base;
				l = list_insert_head(l, target);
				while(graph[target].parent != -10)
				{
					l = list_insert_head(l, graph[target].parent);
					target = graph[target].parent;
					mosse++;
				}

				if(mosse <= mosse_residue)
				{
					list to_return = l->next;
					list_free(q);
					free(graph);
					free(l);
					assigned_flag[end_node] = 1;
					return to_return;
				}
				else
				{
					list_free(q);
					list_free(l);
					free(graph);
					return NULL;
				}
			}

			q = list_insert_tail(q, current - base);
		}

		/*figlio sotto*/
		if(current + base < dim && graph[current + base].is_visited == 0)
		{
			graph[current + base].is_visited = 1;
			graph[current + base].parent = current;
			
			if(scacchiera[current + base] == '>' && assigned_flag[current + base] == 0)
			{
				int mosse = 0;
				int end_node = current + base;
				int target = current + base;
				l = list_insert_head(l, target);
				while(graph[target].parent != -10)
				{
					l = list_insert_head(l, graph[target].parent);
					target = graph[target].parent;
					mosse++;
				}

				if(mosse <= mosse_residue)
				{
					list to_return = l->next;
					list_free(q);
					free(graph);
					free(l);
					assigned_flag[end_node] = 1;
					return to_return;
				}
				else
				{
					list_free(q);
					list_free(l);
					free(graph);
					return NULL;
				}
			}

			q = list_insert_tail(q, current + base);
		}

		/*figlio destra*/
		if(current + 1 < dim && ((current + 1) % base > current % base) && graph[current + 1].is_visited == 0)
		{
			graph[current + 1].is_visited = 1;
			graph[current + 1].parent = current;
			if(scacchiera[current + 1] == '>' && assigned_flag[current + 1] == 0)
			{
				int mosse = 0;
				int end_node = current + 1;
				int target = current + 1;
				l = list_insert_head(l, target);
				while(graph[target].parent != -10)
				{
					l = list_insert_head(l, graph[target].parent);
					target = graph[target].parent;
					mosse++;
				}

				if(mosse <= mosse_residue)
				{
					list to_return = l->next;
					list_free(q);
					free(graph);
					free(l);
					assigned_flag[end_node] = 1;
					return to_return;
				}
				else
				{
					list_free(q);
					list_free(l);
					free(graph);
					return NULL;
				}
			}

			q = list_insert_tail(q, current + 1);
		}

		/*figlio sinistra*/
		if(current - 1 >= 0 && ((current - 1) % base < current % base) && graph[current - 1].is_visited == 0)
		{
			graph[current - 1].is_visited = 1;
			graph[current - 1].parent = current;
			if(scacchiera[current - 1] == '>' && assigned_flag[current - 1] == 0)
			{
				int mosse = 0;
				int target = current - 1;
				int end_node = current - 1;
				l = list_insert_head(l, target);
				while(graph[target].parent != -10)
				{
					l = list_insert_head(l, graph[target].parent);
					target = graph[target].parent;
					mosse++;
				}

				if(mosse <= mosse_residue)
				{
					list to_return = l->next;
					list_free(q);
					free(graph);
					free(l);
					assigned_flag[end_node] = 1;
					return to_return;
				}
				else
				{
					list_free(q);
					list_free(l);
					free(graph);
					return NULL;
				}
			}

			q = list_insert_tail(q, current - 1);
		}
	}
	list_free(q);
	free(graph);
	return NULL;
}