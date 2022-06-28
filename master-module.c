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
#include <time.h>
#include "master-module.h"
#include "my_sem_lib.h"
#include "game-module.h"

/* returns the public_config shm id*/
int loadConfigEnv(struct public_envar** public, struct private_envar** private)
{
	int id_config;
	char* env;

	id_config = shmget(IPC_PRIVATE, sizeof(**public), 0600);
	TEST_ERROR;
	*public = shmat(id_config, NULL, 0);
	TEST_ERROR;

	*private = malloc(sizeof(**private));

	LOADENV(*public, SO_NUM_P);
	LOADENV(*public, SO_BASE);
	LOADENV(*public, SO_ALTEZZA);
	LOADENV(*public, SO_N_MOVES);

	LOADENV(*private, SO_NUM_G);
	LOADENV(*private, SO_MAX_TIME);
	LOADENV(*private, SO_FLAG_MIN);
	LOADENV(*private, SO_FLAG_MAX);
	LOADENV(*private, SO_ROUND_SCORE);

	env = getenv("SO_MIN_HOLD_NSEC");
	if(env == NULL)
	{
		printf("ERRORE: variabile d'ambiente SO_ROUND_SCORE non settata\n");
		exit(EXIT_FAILURE);
	}
	else
		(*public)->SO_MIN_HOLD_NSEC = atof(env);

	return id_config;
}

int piazza_bandierine(char* scacchiera, int* bandierine, int dim, int flag_min, int flag_max, int round_score)
{
	int pos;
	int punti, resto;
	int num_bandierine, to_return;
	to_return = num_bandierine = RAND(flag_min, flag_max);

	punti = round_score / num_bandierine;
	resto = round_score % num_bandierine;

	bzero(bandierine, dim * sizeof(int));

	while(num_bandierine > 0)
	{
		while(scacchiera[(pos = RAND(0, dim-1))] != '-');
		scacchiera[pos] = '>';
		if(resto > 0)
		{
			bandierine[pos] = punti + 1;
			resto--;
		}
		else
			bandierine[pos] = punti;
		num_bandierine--;
	}
	return to_return;
}

char bandierine_presenti(char* scacchiera, int dim)
{
	int i;
	for(i = 0; i < dim; i++)
	{
		if(scacchiera[i] == '>')
			return 1;
	}
	return 0;
}

void inizializza_scacchiera(char* scacchiera, int base, int altezza, int id_sem)
{
	int i, j;
	int dim_scacchiera = base * altezza;
	for(i = 0; i < altezza; i++)
	{
		for(j = 0; j < base; j++)
				SCACCHIERA(base, i,j) = '-';
	}

	for(i = 0; i < dim_scacchiera; i++)
	{
		sem_set_val(id_sem, i, 1);
	}
}

void stampaStato(char* scacchiera, int base, int altezza, player* players, int num_players, int round_number)
{
	stampa_scacchiera_minimal(scacchiera, base, altezza);
	stampaMossePunti(players, num_players);
}

void stampaMossePunti(player* players, int num_players)
{
	int i;
	printf("+----------+----------+----------+\n");
	printf("|Giocatore |  Punti   |  Mosse   | \n");
	printf("+----------+----------+----------+\n");
	for(i = 0; i < num_players; i++)
	{
		printf("|%10c|%10d|%10d|\n", players[i].player_char, players[i].points, *players[i].mosse);
	}
	printf("+----------+----------+----------+\n");
}

void stampaMetriche(int round_number, struct public_envar* public, player* players, int num_players, time_t start_time, time_t end_time)
{
	int i;
	int punti_totali = 0;
	int game_time;
	printf("\033[1;32m"); /*set color to bold green*/
	printf("ROUND GIOCATI: %d\n", round_number);
	printf("\033[0m"); /*set color to white*/

	for(i = 0; i < num_players; i++)
	{
		int mosse_utilizzate = public->SO_N_MOVES * public->SO_NUM_P - *players[i].mosse;
		printf("Giocatore %c\t(Mosse utilizzate)/(Mosse totali) = (%d/%d): %f (%d%%)\n",
				players[i].player_char,
				mosse_utilizzate, public->SO_N_MOVES * public->SO_NUM_P,
				((double) mosse_utilizzate / (double) (public->SO_N_MOVES * public->SO_NUM_P)),
				(int) ((double) mosse_utilizzate / (double) (public->SO_N_MOVES * public->SO_NUM_P) * 100));
	}
	printf("\n");
	for(i = 0; i < num_players; i++)
	{
		int mosse_utilizzate = public->SO_N_MOVES * public->SO_NUM_P - *players[i].mosse;
		printf("Giocatore %c\t(Punti ottenuti)/(Mosse utilizzate): %f\n", players[i].player_char,(double) players[i].points / (double) mosse_utilizzate);
	}
	printf("\n");

	for(i = 0; i < num_players; i++)
		punti_totali += players[i].points;

	game_time = difftime(end_time, start_time);

	printf("(Punti totali)/(Tempo totale di gioco) = (%d/%d): %f\n",
			punti_totali, game_time, (double) punti_totali / (double) game_time);
}