#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "game-module.h"

/* stampa una array in forma tabellare */
void stampa_charArr_tabella(char* arr, int base, int altezza) 
{
	int i, j;
	printf("+");
		for(j = 0; j < base; j++)
			printf("---+");
	printf("\n");
	
	for(i = 0; i < altezza; i++)
	{
		printf("|");
		for(j = 0; j < base; j++)
		{
			printf(" %c |", arr[base*i + j]);
		}
		printf("\n");
		
		printf("+");
		for(j = 0; j < base; j++)
			printf("---+");
		printf("\n");
	}
}

/* stampa una scacchiera in modo minimale */
void stampa_scacchiera_minimal(char* arr, int base, int altezza)
{
	int i, j;
	for(i = 0; i < altezza; i++)
	{
		for(j = 0; j < base; j++)
		{
			switch(arr[base*i+j])
			{
				case '-':
					printf("\033[0m"); /*set color to white*/
				break;
				case '>':
					printf("\033[1;31m"); /*set color to bold red*/
				break;
				default:
					printf("\033[1;34m"); /*set color to bold blue*/
			}
			printf("%c", arr[base*i + j]);
		}
		printf("\n");
	}
	printf("\033[0m"); /*set color to white*/
}