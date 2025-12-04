#include "xerrori.h"
#include "functionality.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>


int main(int argc, char *argv[]){

	fprintf(stderr,"==== INIZIO CONTROLLO SU argc E SU argv[] ====\n");
	// Controllo sul numero di file passati da linea di comando
	if(argc != 4)
	{
		xtermina("USO:\tcammini.out nomi.txt grafo.txt numconsumatori",QUI);
	}

	// Converto il numero di thread consumatori da stringa ad intero
	int numeroConsumer = atoi(argv[3]);

	// Controllo sul numero di consumatori, deve essere strettamente
	// maggiore di zero
	if(numeroConsumer <= 0)
	{
		xtermina("ATTENZIONE:\tIl numero di consumatori deve essere > 0",QUI);
	}

	fprintf(stderr,"==== FINE CONTROLLO SU argc E SU argv[] ====\n");




	fprintf(stderr,"==== INIZIO DICHIARAZIONE THREAD GESTORE SEGNALI ====\n");

	// Definisco l'insieme di segnali da gestire con sigwait()
	sigset_t setSignal;
	sigemptyset(&setSignal);
	sigaddset(&setSignal,SIGINT);

	// Blocco/maschero i segnali
	int errSignalMask = pthread_sigmask(SIG_BLOCK,&setSignal,NULL);
	
	// Controllo se il valore di ritono: se è 0 è andato tutto a buon fine
	// altrimenti c'è un errore
	if(errSignalMask != 0)
	{
		xtermina("ERRORE: errore nella pthread_sigmask",QUI);
	}

	// Creazione dello stato per la gestione dei segnali (dati thread gestore segnali)
	//pthread_mutex_t mutexSignal = PTHREAD_MUTEX_INITIALIZER;
	//xpthread_mutex_init(&mutexSignal,NULL,QUI);

	atomic_int stato_pipe = ATOMIC_VAR_INIT(0);
	atomic_int stato_terminazione = ATOMIC_VAR_INIT(0);
	datiThreadSignal datiTS;
	datiTS.attiva_lettura_pipe = &stato_pipe;
	datiTS.stato_terminazione = &stato_terminazione;

	// Creo il thread per la gestione dei segnali e lo avvio
	pthread_t signalThread;
	xpthread_create(&signalThread,NULL,threadSignalBody,&datiTS,QUI);

	fprintf(stderr,"==== FINE DICHIARAZIONE THREAD GESTORE SEGNALI ====\n");




	fprintf(stderr,"==== INIZIO LETTURA FILE nomi.txt ====\n");

	// Apertura file nomi.txt per lettura
	FILE *fileNomi = xfopen(argv[1],"r",QUI);

	// Inizializzazione e dichiarazione della capacità
	// dell'array di attori
	int capacity = 0;

	// Creazione array di attori
	attore *arrayGrafo = inserimentoAttori(fileNomi, &capacity);

	// Chiudo il file nomi.txt aperto in lettura
	if(fclose(fileNomi) == EOF)
	{
		xtermina("ERRORE: errore chiusura file nomi.txt",QUI);
	}

	fprintf(stderr,"==== FINE LETTURA FILE nomi.txt, PRODOTTO ARRAY DI ATTORI ====\n");





	fprintf(stderr,"==== INIZIO LETTURA FILE grafo.txt ====\n");

	// Apertura file grafo.txt in lettura
	FILE *fileGrafo = xfopen(argv[2],"r",QUI);

	completamentoInserimentoAttori(fileGrafo,arrayGrafo,&capacity,numeroConsumer);

	// Chiudo il file grafo.txt aperto in lettura
	if(fclose(fileGrafo) == EOF)
	{
		xtermina("ERRORE: errore chiusura file grafo.txt",QUI);
	}

	fprintf(stderr,"==== FINE LETTURA FILE grafo.txt ====\n");

	


	fprintf(stderr,"==== INIZIO DICHIARAZIONE PIPE ====\n");

	// Creo la pipe FIFO (speciale tipo di pipe che permette la comunicazione
	// tra processo non correlati)
	int errPipe = mkfifo("./cammini.pipe",0666);

	// Controlli sulla coretta apertura della pipe
	if(errPipe == 0)
	{
		fprintf(stderr,"Crezione pipe\n");
	}
	else if(errno == EEXIST)
	{
		fprintf(stderr,"Pipe già creata\n");
	}
	else
	{
		xtermina("ERRORE: errore creazione pipe\n",QUI);
	}

	// Apro la pipe in sola lettura (fd sta per file descriptor)
	int fd = open("./cammini.pipe",O_RDONLY);

	// Aperta la pipe dico al thread gestore di segnali
	// che se arriva, da ora in poi, un segnale di SIGINT allora
	// può avviare la procedura di terminazione
	atomic_store(datiTS.attiva_lettura_pipe,1);

	camminiMinimiThreadCreate(fd,&stato_terminazione,arrayGrafo,capacity,&signalThread);

	// A questo punto ho letto tutti i valori da leggere e la pipe è stata chiusa
	fprintf(stderr, "== TERMINAZIONE SENZA ALCUN INTOPPO (no seganali) ==\n");

	// Informiamo il thread gestore dei segnali che il programma è terminato e quindi
	// può terminare pure lui
	pthread_kill(signalThread, SIGINT);

	signalDeallocation(arrayGrafo,capacity,&signalThread);

	return 0;
}