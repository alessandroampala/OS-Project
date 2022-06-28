#ifndef PLAYER_MODULE_H
#define PLAYER_MODULE_H
#include "game-module.h"

#define TROPPE_PEDINE 	printf("Troppe pedine per una scacchiera così piccola!\n");\
						exit(EXIT_FAILURE);

/*
 * Piazza una pedina ogni freq celle ogni volta che viene invocata.
 * INPUT:
 * - scacchiera: puntatore all'array usato per rappresentare la scacchiera
 * - n_moves: numero di mosse per pedina
 * - freq: ogni quante celle va posizionata una pedina
 * - dim: dimensione di scacchiera
 * - identifier: carattere ASCII che identifica la pedina
 * - sem_id: array di semafori di dimensione pari a quella di scacchiera
 * RISULTATO
 * - viene tentato il posizionamento di una pedina ogni freq/6 celle
 * - se non ci sono posto assegnato alla pedina è già occupato, si tenta di posizionare
 *   la pedina nella cella adiacente a destra
 * - se ci sono troppe pedine il processo stampa l'errore e termina
 * - viene ritornata una struttura di tipo pedina con le caratteristiche
 *   della pedina appena piazzata
 */
pedina piazza_pedina(char* scacchiera, int n_moves, int freq, int dim, char indentifier, int sem_id);

#endif