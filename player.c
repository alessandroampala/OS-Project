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
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include "game-module.h"
#include "pathfinding.h" 
#include "my_sem_lib.h"

/* id memoria condivisa contenente un array di tipo struct pedina
   contenente le informazioni sulle pedine */
int id_pedine;
pedina* pedine;
struct public_envar * config; /* variabili di configurazione */
char* scacchiera; /*puntatore alla scacchiera */
int master_msgq_id; /* message queue per scambio di messaggi col master */
int pedine_msgq_id; /* message queue per scambio di messaggi con le pedine */
struct msgb message; /* struttura per ricezione generici messaggi */
int id_sem; /* semaforo scacchiera */

/* maschera per tenere conto di quali pedine
   hanno ricevuto informazioni accessibile solo dal giocatore*/
char* indication_received_mask;
int id_player_sem; /* semaforo per la sincronizzazione round */

/* maschera in memoria condivisa per tenere conto 
   di quali pedine hanno ricevuto informazioni.
   Accessibile da tutte le pedine
*/
int id_flag_assigned_mem;
char* flag_assigned_mem;

int main(int argc, char const *argv[])
{
	int i, a, status;
	/* dimensione scacchiera e
	   frequenza secondo cui disporre le pedine */
	int dim, freq;
	/* variabile in shared memory per la gestione delle fasi del gioco */
	char* game_status;

	int count_dormienti = 0;

	/* apertura vari IPCs e operazioni preliminari */
	config = shmat(atoi(argv[1]), NULL, SHM_RDONLY);
	TEST_ERROR;

	dim = config->SO_BASE*config->SO_ALTEZZA;
	freq = dim/config->SO_NUM_P;

	scacchiera = shmat(atoi(argv[2]), NULL, 0);
	TEST_ERROR;
	master_msgq_id = atoi(argv[3]);
	id_sem = atoi(argv[5]);
	game_status = shmat(atoi(argv[9]), NULL, SHM_RDONLY);
	id_pedine = shmget(IPC_PRIVATE, config->SO_NUM_P * sizeof(pedina), 0600);
	pedine = shmat(id_pedine, NULL, 0);
	pedine_msgq_id = msgget(IPC_PRIVATE, 0600);
	indication_received_mask = malloc(config->SO_NUM_P);
	id_player_sem = atoi(argv[11]);
	id_flag_assigned_mem = shmget(IPC_PRIVATE, dim, 0600);
	flag_assigned_mem = shmat(id_flag_assigned_mem, NULL, 0);

	/* azzeramento maschera indicazioni pubblica */
	bzero(&flag_assigned_mem, sizeof(flag_assigned_mem));

	signal(SIGPIPE, SIG_IGN); /* Ignora i segnali di tipo SIGPIPE in caso provassi a scrivere su una pipe di una pedina terminata */

	/* Creazione processi pedina e inizializzazione relative variabili */
	for(i = 0; i < config->SO_NUM_P; i++)
	{
		int j;
		/* attendi che il master mandi un messaggio: quando si riceve un messaggio
		   di tipo metti_pedina vuol dire che tocca a questo giocatore piazzare
		   una pedina */
		msgrcv(master_msgq_id, &message, sizeof(message) - sizeof(long), metti_pedina, 0);

		/*piazza pedina*/
		pedine[i] = piazza_pedina(scacchiera, config->SO_N_MOVES,freq, dim, argv[4][0], id_sem);

		if(!(pedine[i].pid = fork()))
		{
			/* Creazione buffer per passaggio argomenti al processo pedina */
			char* arg[16];
			char positionbuffer[ARGBUFFERSIZE];
			char id_pedine_buffer[ARGBUFFERSIZE];
			char index_pedine_buffer[ARGBUFFERSIZE];
			char pedine_msgq_id_buffer[ARGBUFFERSIZE];
			char id_flag_assigned_membuf[ARGBUFFERSIZE];

			bzero(&positionbuffer, sizeof(positionbuffer));
			bzero(&id_pedine_buffer, sizeof(id_pedine_buffer));
			bzero(&index_pedine_buffer, sizeof(index_pedine_buffer));
			bzero(&pedine_msgq_id_buffer, sizeof(pedine_msgq_id_buffer));
			bzero(&id_flag_assigned_membuf, sizeof(id_flag_assigned_membuf));

			sprintf(positionbuffer, "%d", pedine[i].position);
			sprintf(id_pedine_buffer, "%d", id_pedine);
			sprintf(index_pedine_buffer, "%d", i);
			sprintf(pedine_msgq_id_buffer, "%d", pedine_msgq_id);
			sprintf(id_flag_assigned_membuf, "%d", id_flag_assigned_mem);

			arg[0] = "./pedina";
			arg[1] = (char*) argv[1]; /*config*/
			arg[2] = (char*) argv[2]; /*scacchiera_id*/
			arg[3] = (char*) argv[5]; /*sem_id*/
			arg[4] = id_pedine_buffer;
			arg[5] = index_pedine_buffer;
			arg[6] = (char*) argv[4];
			arg[7] = (char*) argv[6]; /* pointsq_id */
			arg[8] = (char*) argv[7]; /* index points_sem */
			arg[9] = (char*) argv[8]; /*mosse_id */
			arg[10] = (char*) argv[9]; /*id_game_status */
			arg[11] = (char*) argv[10]; /*id_round_sem */
			arg[12] = pedine_msgq_id_buffer;
			arg[13] = (char*) argv[12]; /* id_bandiere_sem */
			arg[14] = id_flag_assigned_membuf;
			arg[15] = NULL;

			/* Chiusura write end della pipe da parte della pedina */
			for(j = 0; j < config->SO_NUM_P; j++)
			{
				close(pedine[j].pipe_fd[1]);
			}

			free(indication_received_mask);

			/* execve sull'eseguibile pedina */
			execve("./pedina", arg, NULL);
			TEST_ERROR;
		}

		/* chiusura read end della pipe della pedina
		   da parte del player */
		close(pedine[i].pipe_fd[0]);

		/* informa il master di aver piazzato una pedina */
		message.mtype = pedina_piazzata;
		msgsnd(master_msgq_id, &message, sizeof(message) - sizeof(long), IPC_NOWAIT);
		TEST_ERROR;
	}

	/*INIZIO ROUNDS */
	while(*game_status)
	{
		/* resetta assegnamento bandierine alle pedine */
		for(i = 0; i < config->SO_NUM_P; i++)
		{
			pedine[i].bandierina_assegnata = -1;
		}

		/* sblocca tutte le pedine che nel round precedente
		   non dovevano muoversi */
		for(i = 0; i < count_dormienti; i++)
		{
			message.mtype = sveglia;
			msgsnd(pedine_msgq_id, &message, 0, 0);
		}
		count_dormienti = 0;

		/* ricevi messaggio dal master:
		   - se message.mtype == fornisci_indicazioni allora il gioco continua
		   - se message.mtype == end allora termina
		*/
		msgrcv(master_msgq_id, &message, sizeof(message) - sizeof(long), 0, 0); 

		if(message.mtype == end)
		{
			break;
		}

		if(*game_status == ROUND_END)
		{
			list assegnate = NULL; /* lista contenente le pedine assegnate a una bandierina */
			bzero(indication_received_mask, config->SO_NUM_P);
			/*assegna una bandierina ad una pedina e dai indicazioni a quella pedina*/
			for(i = 0; i < dim; i++)
			{
				/* Quando si trova una bandierina, cerca
				   la pedina più vicina e fornisci indicazioni
				*/
				if(scacchiera[i] == '>')
				{
					nearest_paw np;
					int j;
					int num_bytes = 0;
					int* buf;
					int lunghezza;
					list l;
					/* dopo questa istruzione np contiene informazioni sulla pedina più vicina alla bandierina */
					np = find_nearest_paw(i, scacchiera, config->SO_BASE, config->SO_ALTEZZA,
											pedine, config->SO_NUM_P, argv[4][0]);
					
					if(np.index != -1 && pedine[np.index].mosse_residue >= lunghezza && !list_contains(assegnate, np.index))
					{
						pedine[np.index].bandierina_assegnata = 1;
						indication_received_mask[np.index] = 1;
						/* calcola la dimensione del path in byte */
						num_bytes = list_size(np.path);
						/* alloca buffer per il percorso */
						buf = malloc(num_bytes);
						l = np.path;
						/* lunghezza del percorso */
						lunghezza = num_bytes / sizeof(num_bytes);
						
						/* caricamento del percorso sul buffer */
						for(j = 0; j < lunghezza; j++)
						{
							buf[j] = l->value;
							l = l->next;
						}

						/* carica il percorso sulla pipe */
						if(pedine[np.index].mosse_residue)
						{
							write(pedine[np.index].pipe_fd[1], &num_bytes, sizeof(num_bytes));
							write(pedine[np.index].pipe_fd[1], buf, num_bytes);
						}
						
						free(buf);
						assegnate = list_insert_head(assegnate, np.index);
					}
					list_free(np.path);
				}
			}

			list_free(assegnate);

			/* informa le pedine che non devono muoversi scrivendo sulla
			   rispettiva pipe */
			for(i = 0; i < config->SO_NUM_P; i++)
			{
				int num_bytes = 0;
				if(!indication_received_mask[i])
				{
					write(pedine[i].pipe_fd[1], &num_bytes, sizeof(num_bytes));
					count_dormienti++;
				}
			}

			/* aspetta che le pedine ricevano informazioni */
			for(i = 0; i < config->SO_NUM_P; i++)
			{
				if(pedine[i].mosse_residue)
					msgrcv(pedine_msgq_id, &message, sizeof(message) - sizeof(long), info_ricevute, 0);
			}

			/* informa il master di aver terminato la comunicazione di informazioni
			   alle pedine
			*/
			message.mtype = ready;
			msgsnd(master_msgq_id, &message, sizeof(message) - sizeof(long), 0);
			/*attendi di essere sbloccato dal master per il prossimo round
			  per evitare di leggere il messaggio appena inviato su master_msgq_id */
			sem_reserve(id_player_sem, 0);
		}
	}
	/*FINE ROUNDS */

	/* aspetta la terminazione delle pedine*/
	for(i=0; i < config->SO_NUM_P; i++)
	{
		int num_bytes = 0;
		if(!pedine[i].mosse_residue)
			write(pedine[i].pipe_fd[1], &num_bytes, sizeof(num_bytes));
	}

	while((a = wait(&status)) != -1);

	free(indication_received_mask);
	/* rimuovi IPCs utilizzati */
	msgctl(pedine_msgq_id, IPC_RMID, NULL);
	shmctl (id_pedine, IPC_RMID, NULL);
	shmdt(pedine);
	shmctl(id_flag_assigned_mem, IPC_RMID, NULL);
	shmdt(flag_assigned_mem);
	return 0;
}