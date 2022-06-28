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
#include <time.h>
#include "game-module.h"
#include "my_sem_lib.h"
#include "pathfinding.h"
#include "list-module.h"

/* struttura variabili di configurazione */
struct public_envar * config;
char* scacchiera; /* scacchiera */
int id_sem;	/* semafori celle scacchiera */

/* file descriptor da cui leggere il percorso
   comunicato dal player tramite pipe */
int directions_fd;
/*carattere che identifica questa pedina
  sulla scacchiera */
char identifier;

int id_round_sem; /* semaforo per sincronizzazione round */
int index_sem; /* indice del player di questa pedina */

/* msq su cui mandare messaggi di cattura bandierina */
int pointsq_id;

/* struct che definisce quanto aspettare prima
   di ricalcolare un nuovo percorso quando si 
   trova un ostacolo sul percorso attuale */
struct timespec timeout;
/* struct utilizzata nella nanosleep
   quando si riesce ad accedere ad una nuova cella */
struct timespec req;

int main(int argc, char** argv)
{
	int bytes_to_read = 0;
	int* path = NULL;
	list path_list = NULL;
	list l = NULL;
	/* lunghezza del percorso che si riceve da pipe */
	int path_length;
	/* ultimo elemento del percorso attuale */
	int last_element;
	/* variabile in memoria condivisa per lo stato di gioco */
	char* game_status;
	/* mosse totali del player in shm */
	unsigned int* mosse_player;
	/* puntatore alla struttura di questa pedina */
	pedina* this;
	/* indice di questa pedina nell'array di pedine */
	int index_pedina;
	/* msgq per scambio informazioni con il player */
	int pedine_msgq_id;
	/* semaforo decrementato ogni volta che si conquista una bandierina */
	int id_bandiere_sem;
	/* puntatore alla maschera di bandierine assegnate */
	char* flag_assigned_mem;

	/* Operazioni preliminari e ottenimento IPCs */
	config = shmat(atoi(argv[1]), NULL, SHM_RDONLY);
	scacchiera = shmat(atoi(argv[2]), NULL, 0);
	id_sem = atoi(argv[3]);
	index_pedina = atoi(argv[5]);
	this = (pedina*) shmat(atoi(argv[4]), NULL, 0) + index_pedina;
	directions_fd = this->pipe_fd[0];

	identifier = *argv[6];
	pointsq_id = atoi(argv[7]);
	index_sem = atoi(argv[8]);
	mosse_player = shmat(atoi(argv[9]), NULL, 0);
	game_status = shmat(atoi(argv[10]), NULL, SHM_RDONLY);
	id_round_sem = atoi(argv[11]);
	pedine_msgq_id = atoi(argv[12]);

	id_bandiere_sem = atoi(argv[13]);
	flag_assigned_mem = shmat(atoi(argv[14]), NULL, 0);

	timeout.tv_sec = 0;
	timeout.tv_nsec = 500; /* set waiting time per gli ostacoli sul percorso */
	req.tv_sec = 0;
	req.tv_nsec = config->SO_MIN_HOLD_NSEC; /* waiting time nanosleep */

	/* INIZIO ROUNDS */
	while(this->mosse_residue && *game_status)
	{
		/* se il round è finito, ricevi informazioni
		   sul percorso dal player */
		if(*game_status == ROUND_END)
		{
			struct flag_msg message;
			message.mtype = info_ricevute;

			read(directions_fd, &bytes_to_read, sizeof(bytes_to_read));

			path_length = bytes_to_read / sizeof(bytes_to_read);
			path = calloc(path_length, sizeof(int));
			read(directions_fd, path, bytes_to_read);

			free(path_list);
			path_list = array_to_list(path, path_length);
			l = path_list;

			if(l != NULL)
				last_element = get_last_element(l, l->value);

			free(path);
			msgsnd(pedine_msgq_id, &message, 0, 0);
			sem_reserve(id_round_sem, 0); /* wait for the next round */

			/* se bytes_to_read == 0, allora questa pedina non deve muoversi:
			   aspetta finchè il player manda un messaggio sbloccante */
			if(!bytes_to_read)
			{
				msgrcv(pedine_msgq_id, &message, 0, sveglia, 0);
			}
		}
		
		/* in caso non ci fosse un percorso da seguire, cerca
		   la bandierina non assegnata più vicina e calcola il percorso */
		if(l == NULL && bytes_to_read && *game_status == ROUND_START)
		{
			list_free(path_list);
			path_list = (find_nearest_flag(this->position, scacchiera, config->SO_BASE, config->SO_ALTEZZA, this->mosse_residue, flag_assigned_mem));
			l = path_list;

			if(l != NULL)
				last_element = get_last_element(l, l->value);
		}

		/* quando il round è in corso, si hanno ancora mosse ed un percorso da seguire,
		   si cerca di continuare con il percorso attuale */
		while(l != NULL && this->mosse_residue && *game_status == ROUND_START)
		{
			char sem_not_obtained = 0;
			errno = 0;
			/* se c'è un ostacolo prova ad aspettare che si sposti,
			   sennò ottieni subito l'accesso al semaforo */
			sem_reserve_timed(id_sem, l->value, &timeout);

			/* accesso al semaforo non ottenuto,
			   l'ostacolo non si è spostato */
			if(errno == EAGAIN)
				sem_not_obtained = 1;

			/* se la bandierina che si sta provando a raggiungere non è ancora stata catturata,
			   allora continua, sennò annulla il percorso attuale */
			if(scacchiera[last_element] == '>')
			{
				/* se non si è ottenuto l'accesso al semaforo, ricalcola il percorso per aggirare l'ostacolo
				   altrimenti controlla se si è raggiunta una bandierina */
				if(sem_not_obtained)
				{
					/* calcola  nuovo percorso con bfs */
					list_free(path_list);
					path_list = bfs(this->position, last_element, scacchiera, config->SO_BASE, config->SO_ALTEZZA);
					l = path_list;
					
					if(l != NULL)
					last_element = get_last_element(l, l->value);
				}
				else
				{
					char isFlag = 0;
					int old_pos = this->position;
					this->position = l->value;
					
					/* operazioni prima di nanosleep per non lasciare
					   lo stato della scacchiera inconsistente */
					if(scacchiera[this->position] == '>') isFlag = 1;
					scacchiera[old_pos] = '-';
					scacchiera[this->position] = identifier;					
					sem_release(id_sem, old_pos); /* rilascia semaforo cella precedente */					
					nanosleep(&req, NULL);

					/* se si è raggiunta una bandierina, comunicare al master
					   la posizione della bandierina raggiunta e l'index del player */
					if(isFlag)
					{
						struct flag_msg message;
						/* + 1 perchè i messaggi non possono avere tipo == 0 */
						message.mtype = this->position + 1;
						message.player_index = index_sem;
						sem_reserve(id_bandiere_sem, 0);
						msgsnd(pointsq_id, &message, sizeof(message) - sizeof(long), 0);
						flag_assigned_mem[this->position] = 0;
					}
					/* decrementa mosse player */
					(*mosse_player)--;
					/* decrementa mosse pedina */
					(this->mosse_residue)--;
					/* setta nuova cella da raggiungere */
					l = l->next;
				}
			}
			else
			{
				if(!sem_not_obtained)
					sem_release(id_sem, l->value);
				list_free(path_list);
				path_list = NULL;
				l = NULL;
			}
		}		
	}
	/* FINE ROUNDS */
	return 0;
}