#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include "my_sem_lib.h"
#include "player-module.h"

pedina piazza_pedina(char*scacchiera, int n_moves, int freq, int dim, char indentifier, int sem_id)
{
	static int cella = 0;
	static int disp = 1;
	pedina p;
	static int iterations = 0;

	while(scacchiera[cella] != '-')
	{
		cella += freq - freq / 6;
		if(cella >= dim)
		{
			cella = disp++;
			if(disp >= (freq - freq / 6) + 1)
			{
				TROPPE_PEDINE;
			}
		}

		if(iterations++ >= dim*2 )
		{
			printf("TROPPE_PEDINE\n");
			TROPPE_PEDINE;
		}
	}
	scacchiera[cella] = indentifier;
	p.position = cella;
	p.bandierina_assegnata = -1;
	p.mosse_residue = n_moves;
	pipe(p.pipe_fd);
	sem_reserve(sem_id, cella);
	return p;
}