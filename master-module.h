#ifndef _MASTER_MODULE_H
#define _MASTER_MODULE_H
#include "game-module.h"

/* macro per il caricamento della variabile d'ambiente name nella struttura config */
#define	LOADENV(config, name)	{															\
						char* env;															\
						env = getenv(#name);												\
						if(env == NULL)														\
						{																	\
							printf("ERRORE: variabile d'ambiente %s non settata\n", #name);	\
							exit(EXIT_FAILURE);												\
						}																	\
						else																\
						{																	\
							(config)-> name = atoi(env);									\
						}																	\
						}

#define SCACCHIERA(base, i, j) scacchiera[base*i + j]

typedef struct player
{
	int pid;
	char player_char;
	int msgq_id;
	int points;
	int mosse_id;
	unsigned int* mosse;
} player;

/*
 * Carica le variabili di configurazione di gioco dalle
 * variabili d'ambiente.
 * INPUT:
 * - public: indirizzo di un puntatore ad una struttura di tipo public_envar
 *   NON precedentemente allocata
 * - private: indirizzo di un puntatore ad una struttura di tipo private_envar
 *   NON precedentemente allocata
 * RISULTATO
 * - la struttura private viene allocata nell'heap tramite malloc
 * - la struttura public viene allocata in memoria condivisa
 *   con permessi 0600 e viene effettuata una shmat
 * - le variabili di configurazione SO_NUM_P,SO_BASE,SO_ALTEZZA,SO_N_MOVES
 *   vengono caricate nella struttura public
 * - le variabili SO_NUM_G,SO_MAX_TIME,SO_FLAG_MIN,SO_FLAG_MAX,SO_ROUND_SCORE
 *   vengono caricate nella struttura private
 * - il valore di ritorno è l'id della struttura public in memoria condivisa
 */
int loadConfigEnv(struct public_envar** public, struct private_envar** private);

/* Piazza un numero random compreso tra flag_min e flag_max di bandierine
 * sulla scacchiera.
 * INPUT:
 * - scacchiera: puntatore all'array usato per rappresentare la scacchiera
 * - bandierine: array della stessa dimensione di scacchiera dove sono contenuti i punteggi
 *   delle singole bandierine
 * - dim: numero di celle della scacchiera
 * - flag_min: numero minimo di bandierine possibili
 * - flag_max: numero massimo di bandierine possibili
 * - round_score: punteggio totale assegnato per round alle varie bandierine
 * RISULTATO
 * - la scacchiera viene popolata di bandierine
 * - il valore di ritorno è il numero di bandierine piazzate
 */
int piazza_bandierine(char* scacchiera, int* bandierine, int dim, int flag_min, int flag_max, int round_score);

/* Setta tutte le celle dell'array puntato da scacchiera a '-'
 * e setta il semaforo corrispondente a quella cella a 1.
 * INPUT:
 * - scacchiera: puntatore all'array usato per rappresentare la scacchiera
 * - dim: numero di celle della scacchiera
 * RISULTATO:
 * - ritorna 1 se la scacchiera contiene almeno una bandierina
 * - ritorna 0 se la scacchiera non contiene bandierine
 */
char bandierine_presenti(char* scacchiera, int dim);

/* Setta tutte le celle dell'array puntato da scacchiera a '-'
 * e setta il semaforo corrispondente a quella cella a 1.
 * INPUT:
 * - scacchiera: puntatore all'array usato per rappresentare la scacchiera
 * - base: dimensione della base della scacchiera
 * - altezza: dimensione dell'altezza della scacchiera
 * - id_sem: id del semaforo associato alla scacchiera
 */
void inizializza_scacchiera(char* scacchiera, int base, int altezza, int id_sem);

/* Stampa stato del gioco:
 * - una rappresentazione visuale della scacchiera in ASCII
 * - il punteggio attuale dei giocatori e le mosse residue a disposizione
 * INPUT:
 * - scacchiera: puntatore all'array usato per rappresentare la scacchiera
 * - base: dimensione della base della scacchiera
 * - altezza: dimensione dell'altezza della scacchiera
 * - players: puntatore ad una struttura player contenente le info dei players
 * - num_players: numero di players
 * - round_number: numero di round
 */
void stampaStato(char* scacchiera, int base, int altezza, player* players, int num_players, int round_number);

/* Stampa mosse e punti dei giocatori in forma tabellare
 * INPUT:
 * - players: puntatore ad una struttura player contenente le info dei players
 * - num_players: numero di players
 */
void stampaMossePunti(player* players, int num_players);

/* Stampa le metriche di fine gioco:
 * - numero di round giocati
 * - rapporto tra mosse utilizzate e mosse totali per ciascun giocatore
 * - rapporto tra punti ottenuti e mosse utilizzate per ciascun giocatore
 * - rapporto tra punti totali (sommati fra quelli di tutti i giocatori) e tempo totale di gioco
 * INPUT:
 * - round_number: numero di round giocati
 * - public: puntatore a struttura di tipo struct public_envar
 * - players: puntatore a struttura di tipo player
 * - num_players: numero di giocatori
 * - start_time: variabile time_t rappresentate il momento in cui ha inizio il gioco
 * - end_time: variabile time_t rappresentate il momento in cui il gioco finisce
 */
void stampaMetriche(int round_number, struct public_envar* public, player* players, int num_players, time_t start_time, time_t end_time);

#endif