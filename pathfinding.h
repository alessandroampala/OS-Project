#ifndef PATHFINDING_H
#define PATHFINDING_H
#include "list-module.h"
#include "player-module.h"

/* struttura per tenere traccia delle proprietà
 * dei nodi del grafo durante gli algoritmi di
 * pathfinding utilizzati
 */
typedef struct path_node
{
	char is_visited;
	int parent;
} path_node;

/* struttura utilizzata per
 * ritornare l'indice della pedina
 * nell'array pedine e il percorso
 * da comunicare a quella pedina
 */
typedef struct nearest_paw
{
	int index;
	list path;
} nearest_paw;

/* Restituisce l'indice della pedina
 * in posizione pos_pedina.
 * INPUT:
 * - pos_pedina: la posizione della pedina di cui si
 *   vuole conoscere l'indice
 * - pedine: puntatore ad array di tipo pedina
 * - num_pedine: numero di pedine nell'array pedine
 * RISULTATO
 * - se esiste una pedina in posizione pos_pedina,
 *   il valore di ritorno è l'indice di quella pedina
 *   nell'array pedine
 * - se non esiste una pedina in posizione pos_pedina
 *   viene ritornato -1
 */
int index_pedina(int pos_pedina, pedina* pedine, int num_pedine);

/* Restituisce 1 se la pedina in pos_pedina è già stata assegnata
 * ad una bandierina, 0 altrimenti
 * INPUT:
 * - pos_pedina: la posizione della pedina di cui si
 *   vuole conoscere l'indice
 * - pedine: puntatore ad array di tipo pedina
 * - num_pedine: numero di pedine nell'array pedine
 */
int pedina_assegnata(int pos_pedina, pedina* pedine, int num_pedine);

/* Implementazione dell'algoritmo BFS per trovare il percorso minimo
 * dal nodo start al nodo goal.
 * INPUT:
 * - start: start node da cui iniziare la ricerca del percorso
 * - goal: il nodo da raggiungere
 * - scacchiera: la scacchiera su cui devono esistere i nodi start e goal,
 *   e su cui si vuole cercare il percorso minimo
 * - base: la base della scacchiera
 * - altezza: l'altezza della scacchiera
 * RISULTATO
 * - se esiste un percorso minimo da start a goal, viene ritornato
 *   sotto forma di list
 * - se non esiste, viene ritornato NULL
 */
list bfs(int start, int goal, char* scacchiera, int base, int altezza);

/* Data la posizione di una bandierina, trova la pedina più vicina
 * e il percorso minimo per raggiungere quella pedina.
 * Viene utilizzata una variante di BFS.
 * INPUT:
 * - start_flag: la posizione della badierina
 * - scacchiera: la scacchiera di riferimento
 * - base: la base della scacchiera
 * - altezza: l'altezza della scacchiera
 * - pedine: puntatore ad array di tipo pedina
 * - num_pedine: numero di pedine nell'array pedine
 * - identifier: carattere che identifica l'insieme di pedine da trovare
 * RISULTATO
 * - se esiste una pedina con abbastanza mosse residue per raggiungere la bandierina,
 *   allora ritorna una struttura nearest_paw con index che indica l'indice della pedina
 *   nell'array pedine e path una lista che indica il percorso che la pedina deve fare
 *   per raggiungere quella bandierina
 * - altrimenti ritorna una struttura nearest_paw con index = -1 e path = NULL
 */
nearest_paw find_nearest_paw(int start_flag, char* scacchiera, int base, int altezza,
						pedina* pedine, int num_pedine, char identifier);

/* Data la posizione di una pedina sulla scacchiera, trova la bandierina più vicina ed
 * il percorso per raggiungerla. Viene utilizzata una variante di BFS.
 * INPUT:
 * - start: la posizione iniziale della pedina
 * - scacchiera: la scacchiera di riferimento
 * - base: la base della scacchiera
 * - altezza: l'altezza della scacchiera
 * - mosse_residue: le mosse residue della pedina
 * - assigned_flag: maschera di bandierine già assegnate ad una pedina
 * RISULTATO
 * - se la lunghezza del percorso <= mosse_residue, allora
 *   ritorna il percorso per raggiungere una bandierina sotto forma di list
 *   e assegna quella bandierina alla pedina aggiornando la maschera assigned_flag
 * - altrimenti ritorna NULL
*/
list find_nearest_flag(int start, char* scacchiera, int base, int altezza, int mosse_residue, char* assigned_flag);

#endif