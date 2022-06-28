#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include "master-module.h"
#include "game-module.h"
#include "my_sem_lib.h"

void set_SIGINT_handler(struct sigaction* sa_a);
void set_SIGALRM_handler(struct sigaction* sa);
void remove_IPCs(int signal);
void end_game(int i);
void sigint_handler(int i);

struct sigaction sa;

int id_scacchiera; /* id della memoria condivisa della scacchiera */
char* scacchiera; /* puntatore alla scacchiera in memoria condivisa */
int dim_scacchiera; /* dimensione scacchiera */
int id_sem; /* id semaforo celle scacchiera */

int id_public_config; /* id struttura variabili di configurazione pubbliche */
/* puntatore alla struttura contenente le variabili di configurazione pubbliche
   in memoria condivisa
*/
struct public_envar* pub_config;
/* puntatore alla struttura contenente le variabili di configurazione private
   allocate nell'heap
*/
struct private_envar* priv_config;

/* struttura per ricezione generici messaggi */
struct msgb message;

/* puntatore ad array in memora condivisa che
   contiene i punteggi delle bandierine
*/
int* bandierine;
int id_bandierine; /* id shared memory */

/* puntatore ad un array di tipo player per la gestione
   dei giocatori
*/
player* players;
/* messagge queue per la gestione della cattura bandierine */
int pointsq_id;
/* variabile in shared memory per la gestione delle fasi del gioco */
char* game_status;
int id_game_status; /* id shared memory */

/* id semaforo per gestione round pedine */
int id_round_sem;
/* id semaforo per la gestione round players */
int id_player_sem;

/* semaforo utilizzato per bloccare il master finchè
   tutte le bandierine del round non vengono prese
*/
int id_bandiere_sem;
int num_bandierine; /* numero bandierine piazzare nel round corrente */
int round_number; /* numero di round giocati */

/* per tenere conto del momento in cui inizia e
   del momento in cui finisce il gioco */
time_t start_game_time, end_game_time;

int main()
{
	int i;
	char current_player_char = 65;
	num_bandierine = 0;
	round_number = 0;

	/* carica configurazione di gioco da variabili d'ambiente */
	id_public_config = loadConfigEnv(&pub_config, &priv_config);

	/* I caratteri dispoibili per rappresentare i giocatori vanno da 
	   65 ('A') a 126 ('~') */
	if(priv_config->SO_NUM_G > 62)
	{
		printf("TROPPI GIOCATORI!\n");
		exit(EXIT_FAILURE);
	}

	dim_scacchiera = pub_config->SO_BASE * pub_config->SO_ALTEZZA;
	
	/* creazione IPCs */
	id_scacchiera = shmget(IPC_PRIVATE, dim_scacchiera, 0600); /* crea una scacchiera di byte con permessi rw per user */
	TEST_ERROR;
	scacchiera = shmat(id_scacchiera, NULL, 0);
	TEST_ERROR;
	id_bandierine = shmget(IPC_PRIVATE, dim_scacchiera * sizeof(int), 0600);
	TEST_ERROR;
	bandierine = shmat(id_bandierine, NULL, 0);
	TEST_ERROR;
	id_game_status = shmget(IPC_PRIVATE, 1, 0600);
	TEST_ERROR;
	game_status = shmat(id_game_status, NULL, 0);
	TEST_ERROR;
	pointsq_id = msgget(IPC_PRIVATE, 0600);
	id_sem = semget(IPC_PRIVATE, dim_scacchiera, 0600);
	id_bandiere_sem = semget(IPC_PRIVATE, 1, 0600);
	id_round_sem = semget(IPC_PRIVATE, 1, 0600);
	id_player_sem = semget(IPC_PRIVATE, 1, 0600);
	
	*game_status = ROUND_END;

	players = calloc(priv_config->SO_NUM_G, sizeof(player));
	TEST_MALLOC(players);

	sem_set_val(id_player_sem, 0, 0);
	
	/* inizializza scacchiera per l'inizio del gioco */
	inizializza_scacchiera(scacchiera, pub_config->SO_BASE, pub_config->SO_ALTEZZA, id_sem);
	
	set_SIGINT_handler(&sa);
	set_SIGALRM_handler(&sa);

	/* generate the players processes */
	for(i = 0; i < priv_config->SO_NUM_G; i++)
	{
		players[i].player_char = current_player_char;
		players[i].msgq_id = msgget(IPC_PRIVATE, 0600);
		TEST_ERROR;
		players[i].mosse_id = shmget(IPC_PRIVATE, sizeof(unsigned int), 0600);
		TEST_ERROR;
		players[i].mosse = shmat(players[i].mosse_id, NULL, 0);
		TEST_ERROR;
		*players[i].mosse = pub_config->SO_NUM_P*pub_config->SO_N_MOVES;
		
		if(!(players[i].pid = fork()))
		{
			/* crea buffer per passaggi argomenti ad execve()*/
			char* arg[14];
			char scacchierabuffer[ARGBUFFERSIZE];
			char configbuffer[ARGBUFFERSIZE];
			char msgkeybuffer[ARGBUFFERSIZE];
			char semidbuffer[ARGBUFFERSIZE];
			char indexpointbuffer[ARGBUFFERSIZE];
			char mosse_idbuf[ARGBUFFERSIZE];
			char pointsqidbuffer[ARGBUFFERSIZE];
			char game_statusbuffer[ARGBUFFERSIZE];
			char id_round_sembuffer[ARGBUFFERSIZE];
			char id_player_sembuffer[ARGBUFFERSIZE];
			char id_bandiere_sembuffer[ARGBUFFERSIZE];

			/* azzera tutti i buffer */
			bzero(&configbuffer, sizeof(configbuffer));
			bzero(&scacchierabuffer, sizeof(scacchierabuffer));
			bzero(&msgkeybuffer, sizeof(msgkeybuffer));
			bzero(&semidbuffer, sizeof(semidbuffer));;
			bzero(&indexpointbuffer, sizeof(indexpointbuffer));
			bzero(&pointsqidbuffer, sizeof(pointsqidbuffer));
			bzero(&game_statusbuffer, sizeof(game_statusbuffer));
			bzero(&mosse_idbuf, sizeof(mosse_idbuf));
			bzero(&id_round_sembuffer, sizeof(id_round_sembuffer));
			bzero(&id_player_sembuffer, sizeof(id_player_sembuffer));
			bzero(&id_bandiere_sembuffer, sizeof(id_bandiere_sembuffer));

			/* copia nei buffer le corrispettive variabili */
			sprintf(configbuffer, "%d", id_public_config);
			sprintf(scacchierabuffer, "%d", id_scacchiera);
			sprintf(msgkeybuffer, "%d", players[i].msgq_id);
			sprintf(semidbuffer, "%d", id_sem);
			sprintf(indexpointbuffer, "%d", i);
			sprintf(pointsqidbuffer, "%d", pointsq_id);
			sprintf(game_statusbuffer, "%d", id_game_status);
			sprintf(id_round_sembuffer, "%d", id_round_sem);
			sprintf(mosse_idbuf, "%d", players[i].mosse_id);
			sprintf(id_player_sembuffer, "%d", id_player_sem);
			sprintf(id_bandiere_sembuffer, "%d", id_bandiere_sem);
			
			arg[0] = "./player";
			arg[1] = configbuffer; /*variabili di configurazione*/
			arg[2] = scacchierabuffer; /*scacchiera in shared memory*/
			arg[3] = msgkeybuffer;	/*id msg queues*/
			arg[4] = &current_player_char;	/* carattere con cui si rappresentano le pedine di questo player*/
			arg[5] = semidbuffer;
			arg[6] = pointsqidbuffer;
			arg[7] = indexpointbuffer;
			arg[8] = mosse_idbuf;
			arg[9] = game_statusbuffer;
			arg[10] = id_round_sembuffer;
			arg[11] = id_player_sembuffer;
			arg[12] = id_bandiere_sembuffer;
			arg[13] = NULL;

			free(players);

			execve("./player", arg, NULL);
			TEST_ERROR;
		}
		current_player_char++;
	}

	for(i = 0; i < pub_config->SO_NUM_P * priv_config->SO_NUM_G; i++)
	{
		message.mtype = metti_pedina;
		msgsnd(players[i % priv_config->SO_NUM_G].msgq_id, &message, sizeof(message) - sizeof(long), IPC_NOWAIT);
		TEST_ERROR;
		msgrcv(players[i % priv_config->SO_NUM_G].msgq_id, &message, sizeof(message) - sizeof(long), pedina_piazzata, 0);
	}
	srand(getpid() + time(0));

	start_game_time = time(NULL);
	/*INIZIO ROUNDS*/
	do
	{
		*game_status = ROUND_END;
		num_bandierine = piazza_bandierine(scacchiera, bandierine, dim_scacchiera,
						priv_config->SO_FLAG_MIN, priv_config->SO_FLAG_MAX, priv_config->SO_ROUND_SCORE);
		
		/*comunica ai giocatori che possono fornire indicazioni ai players*/
		for(i = 0; i < priv_config->SO_NUM_G; i++)
		{
			message.mtype = fornisci_indicazioni;
			msgsnd(players[i].msgq_id, &message, sizeof(message) - sizeof(long), 0);
		}
		/*attendi che tutti i giocatori abbiano finito di dare indicazioni*/
		for(i = 0; i < priv_config->SO_NUM_G; i++)
		{
			msgrcv(players[i].msgq_id, &message, sizeof(message) - sizeof(long), ready, 0);
		}

		/*ora che tutti i messaggi su players[i].msgq_id sono stati letti,
		  sblocca i player */
		sem_set_val(id_player_sem, 0, priv_config->SO_NUM_G);
		/* setta questo semaforo al numero di bandierine piazzate */
		sem_set_val(id_bandiere_sem, 0, num_bandierine);

		system("clear"); /*pulisci il teminale prima di stampare di nuovo lo stato */
		stampaStato(scacchiera, pub_config->SO_BASE, pub_config->SO_ALTEZZA, players, priv_config->SO_NUM_G, round_number);

		/*fai partire le pedine e il timer */
		*game_status = ROUND_START;
		alarm(priv_config->SO_MAX_TIME);
		/* sblocca le pedine per l'inizio del round */
		sem_set_val(id_round_sem, 0, pub_config->SO_NUM_P * priv_config->SO_NUM_G);
		/*attendi che tutte le bandierine siano state catturate */
		sem_wait_for_zero(id_bandiere_sem, 0);
		/* quando tutte le bandierine sono state prese,
		   annulla lo scadere del timer */
		alarm(0);

		/*round terminato*/
		sem_set_val(id_round_sem, 0, 0);
		*game_status = ROUND_END;
		round_number++;

		/*ricevi num_bandierine messaggi dalle pedine quando catturano bandierine
		  e aggiorna il punteggio*/
		for(i = 0; i < num_bandierine; i++)
		{
			struct flag_msg flag_message;
			msgrcv(pointsq_id, &flag_message, sizeof(flag_message) - sizeof(long), 0, 0);
			players[flag_message.player_index].points += bandierine[flag_message.mtype - 1];
			bandierine[flag_message.mtype - 1] = 0;
		}
	}while(1);
	return 0;
}


void set_SIGINT_handler(struct sigaction* sa_a)
{
	bzero(sa_a, sizeof(*sa_a));
	sa_a->sa_handler = sigint_handler;
	sigaction(SIGINT, sa_a, NULL);
}

void set_SIGALRM_handler(struct sigaction* sa)
{
	bzero(sa, sizeof(*sa));
	sa->sa_handler = end_game;
	sigaction(SIGALRM, sa, NULL);
}

/* funzione chiamata quando scade il timer durante un round */
void end_game(int i)
{
	int a;
	int status;
	int val;

	val = semctl(id_bandiere_sem, 0, GETVAL);
	if(val == 0) return;


	*game_status = GAME_END;
	end_game_time = time(NULL);
	/* manda messaggi ai players informandoli della fine del gioco */
	message.mtype = end;
	for(i = 0; i < priv_config->SO_NUM_G; i++)
	{
		msgsnd(players[i].msgq_id, &message, sizeof(message) - sizeof(long), IPC_NOWAIT);
	}

	/* aggiorna punteggi delle ultime bandierine catturate */
	for(i = 0; i < num_bandierine; i++)
	{
		struct flag_msg flag_message;
		errno = 0;
		msgrcv(pointsq_id, &flag_message, sizeof(flag_message) - sizeof(long), 0, IPC_NOWAIT);
		if(errno != ENOMSG)
		{
			players[flag_message.player_index].points += bandierine[flag_message.mtype];
			bandierine[flag_message.mtype] = 0;
		}
	}

	system("clear");
	stampaStato(scacchiera, pub_config->SO_BASE, pub_config->SO_ALTEZZA, players, priv_config->SO_NUM_G, round_number);
	stampaMetriche(round_number, pub_config, players, priv_config->SO_NUM_G, start_game_time, end_game_time);

	while((a = wait(&status)) != -1); /* attendi la terminazione dei players */

	remove_IPCs(0);
	exit(EXIT_SUCCESS);
}

void sigint_handler(int i)
{
	int a;
	int status;

	*game_status = GAME_END;
	end_game_time = time(NULL);
	/* manda messaggi ai players informandoli della fine del gioco */
	message.mtype = end;
	for(i = 0; i < priv_config->SO_NUM_G; i++)
	{
		msgsnd(players[i].msgq_id, &message, sizeof(message) - sizeof(long), IPC_NOWAIT);
	}

	/* aggiorna punteggi delle ultime bandierine catturate */
	for(i = 0; i < num_bandierine; i++)
	{
		struct flag_msg flag_message;
		errno = 0;
		msgrcv(pointsq_id, &flag_message, sizeof(flag_message) - sizeof(long), 0, IPC_NOWAIT);
		if(errno != ENOMSG)
		{
			players[flag_message.player_index].points += bandierine[flag_message.mtype];
			bandierine[flag_message.mtype] = 0;
		}
	}

	system("clear");
	stampaStato(scacchiera, pub_config->SO_BASE, pub_config->SO_ALTEZZA, players, priv_config->SO_NUM_G, round_number);
	stampaMetriche(round_number, pub_config, players, priv_config->SO_NUM_G, start_game_time, end_game_time);

	while((a = wait(&status)) != -1); /* attendi la terminazione dei players */

	remove_IPCs(0);
	exit(EXIT_SUCCESS);
}

void remove_IPCs(int signal)
{
	int i;
	/* marcatura segmenti di memoria condivisa per la deallocazione
	   e detach */
	shmctl (id_scacchiera, IPC_RMID, NULL);
	shmdt(scacchiera);
	shmctl (id_public_config, IPC_RMID, NULL);
	shmdt(pub_config);
	shmctl(id_bandierine, IPC_RMID, NULL);
	shmdt(bandierine);
	shmctl(id_game_status, IPC_RMID, NULL);
	shmdt(game_status);
	/* rimozione semafori */
	semctl(id_sem, 0, IPC_RMID);
	semctl(id_round_sem, 0, IPC_RMID);
	semctl(id_player_sem, 0, IPC_RMID);
	semctl(id_bandiere_sem, 0, IPC_RMID);
	for(i = 0; i < priv_config->SO_NUM_G; i++)
	{
		shmctl(players[i].mosse_id, IPC_RMID, NULL);
		shmdt(players[i].mosse);
	}
	/* rimozione message queues */
	msgctl(pointsq_id, IPC_RMID, NULL);
	for(i = 0; i < priv_config->SO_NUM_G; i++)
	{
		msgctl(players[i].msgq_id, IPC_RMID, NULL);
	}
	free(priv_config);
	free(players);
	exit(EXIT_SUCCESS);
}