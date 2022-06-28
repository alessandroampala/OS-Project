#ifndef _GAME_MODULE_H
#define _GAME_MODULE_H

/* macro per test della variabile errno */
#define TEST_ERROR    if (errno) {dprintf(STDERR_FILENO,		\
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

#define TEST_MALLOC(var) if(var == NULL)								\
						{												\
							printf("Unable to malloc\n");				\
							exit(EXIT_FAILURE);							\
						}

/* Macro per generare numeri random compresi tra min e max */
#define RAND(min, max) min + rand() % (max + 1 - min)

/* 
 * Macro per definire la lunghezza massima delle stringhe utilizzate 
 * per il passaggio argomenti durante la chiamata ad execve()
 */
#define ARGBUFFERSIZE 11

/*define for the shm game_status variable*/
#define GAME_END 0
#define ROUND_END 1
#define ROUND_START 2

/* 
 * struct per variabili di configurazione del gioco
 * accessibili solo al master
 */
struct private_envar
{
	int SO_NUM_G, 
	SO_MAX_TIME,
	SO_FLAG_MIN, 
	SO_FLAG_MAX, 
	SO_ROUND_SCORE;
};

/* 
 * struct per le variabili d'ambiendte
 * accessibili a tutti (master, player, pedina)
 */
struct public_envar
{
	int SO_NUM_P,
	SO_BASE, SO_ALTEZZA,
	SO_N_MOVES;
	long SO_MIN_HOLD_NSEC;
};

/* struct per gestione pedine */
typedef struct pedina
{
	int mosse_residue; /* mosse che la pedina ha ancora a disposizione */
	int position; /* posizione della cella che occupa la pedina */
	int pid;	  /* pid del processo pedina */
	int pipe_fd[2]; /*pipe di comunicazione verso la pedina*/
	int bandierina_assegnata; /*posizione bandierina assegnata */
} pedina;

/* 
 * struct per l'invio di generici messaggi
 * che necessitano solo del tipo
 */
struct msgb
{
	long mtype;
};

/* 
 * struct per l'invio di messaggi di cattura bandierina
 * da un processo pedina al processo master
 */
struct flag_msg
{
	long mtype;
	unsigned int player_index;
};

/* enum che continene i possibili valori di mtype durante l'invio di un messaggio */
enum msg_type {metti_pedina = 1, pedina_piazzata, fornisci_indicazioni, ready, end, ricevo_info, info_ricevute, spostamenti_finiti, sveglia};

/*
 * Stampa un array visto come una tabella con base e altezza passati
 * come parametri.
 */
void stampa_charArr_tabella(char* arr, int base, int altezza);

/*
 * Stampa una scacchiera in forma minimale.
 */
void stampa_scacchiera_minimal(char* arr, int base, int altezza);


#endif