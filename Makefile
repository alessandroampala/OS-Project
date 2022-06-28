# compilatore C da usare
CC = gcc
# flags per la compilazione
CFLAGS = -std=c89 -Wpedantic -Wall

all: player  master  pedina

# con "make clean" solitamente si vogliono eliminare i prodotti della
# compilazione e ripristinare il contenuto originale
clean:
	rm -f *.o master player pedina *~

# per la generazione dei moduli oggetto ci avvaliamo
# delle builtin rules di make che ci consentono di non esplicitare
# altre regole

pedina: pedina.c game-module.h game-module.o my_sem_lib.h my_sem_lib.o list-module.h list-module.o pathfinding.h pathfinding.o Makefile
	$(CC) $(CFLAGS) pedina.c game-module.o my_sem_lib.o list-module.o pathfinding.o -o pedina

player: player.c player-module.h player-module.o game-module.h game-module.o list-module.h list-module.o pathfinding.h pathfinding.o my_sem_lib.h my_sem_lib.o Makefile
	$(CC) $(CFLAGS) player.c player-module.o game-module.o list-module.o pathfinding.o my_sem_lib.o -o player

master: master.c master-module.h master-module.o game-module.h game-module.o my_sem_lib.h my_sem_lib.o Makefile
		$(CC) $(CFLAGS) master.c master-module.o game-module.o list-module.o my_sem_lib.o -o master

# il target run si usa talvolta per eseguire l'applicazione
run:  all
	./master