#include "xerrori.h"
#include <stdatomic.h>
#include <time.h>

// QUi viene inserito all'interno di alcune istruzioni nel file
// xerrori.c (e di conseguenzza anche in xerrori.h). Qui sta ad indicare:
// __LINE__ la linea a cui viene effettuata l'istruzione, mentre __FILE__ il
// file in cui a tale linea viene fatta l'istruzione 
#define QUI __LINE__,__FILE__
#define buffSize 128

// Stuttura per la difinizione di ogni nodo del grafo
// (cioè per la definizione di ogni attore)
typedef struct {
	int codice;		// Codice univo identificativo
	char *nome;
	int anno;
	int numcop;		// Dimensione array dei coprotagonisti
	int *cop;		// Puntatore all'array di coprotagonisti
} attore;


// Struttura per la definizione del thread Producer
typedef struct {
	char **buffer;			// Buffer condiviso
	int *pIndex;			
	sem_t *sem_free_slots;	// Semaforo per il numero di slots liberi
	sem_t *sem_data_items;	// Semaforo per il numero di slots occupati
	FILE *nomeFile;			// Puntatore al file in cui leggere
} datiProducer;


// Struttura per la definizione dei thread Consumer
typedef struct {
	char **buffer;			// Buffer condiviso
	int *cIndex;			
	pthread_mutex_t *mutex;	// Mutex
	sem_t *sem_free_slots;	// Semaforo per il numero di slots occupati
	sem_t *sem_data_items;	// emaforo per il numero di slots occupati
	attore *arrayGrafo;		// Array di attori a cui accedere
	int capacity;			// Capacità dell'array di attori
} datiConsumer;


// Struttura da passare al thread di gestione di segnali
typedef struct{
	atomic_int* attiva_lettura_pipe;
	atomic_int* stato_terminazione;
} datiThreadSignal;


typedef struct{
	int a;				// valore letto dalla pipe
	int b;				// valore letto dalla pipe
	attore *arrayGrafo;	// Array di struct di attori (grafo sottoforma di array)
	int capacity;		// Dimnesione/capacità dell'array di attori
} datiThreadMinPath;


typedef struct nodoABR{
	attore *actor;
	int codActShuffle; 
	struct nodoABR* predecessore;
	struct nodoABR* figlioDx;
	struct nodoABR* figlioSx;
} nodoABR;


typedef struct{
	attore **coda;
	int capacity;
	int codiceTesta;
	int codiceCoda;
} FIFO;


// Funzione che prende come argomenti il puntatore al file e l'indirizzo
// dell'indice, crea un array di struct attori leggendo dal file nomi.txt
// e restituisce l'array di attori al chimante
attore* inserimentoAttori(FILE *nomeFile, int *capacity);


// Funzione che inserisce per ogni attore, all'interno dell'array di attori (del grafo),
// la dimensione dell'array che continene i codici dei coprotagonisti e alloca per ogni
// attore l'array dei coprotagonisti (composto dai codici univoci dei coprotagonisti)
void completamentoInserimentoAttori(FILE *nomeFile, attore *arrayGrafo,int *capacity, int numeroConsumer);


// Funzione che verrà eseguita dal thread produttore:
void *threadProdBody(void *args);


// Funzione che verrà eseguita dai thread consumatori:
void *threadConsBody(void *args);


// Funzione che verrà eseguita dal therad gestore di segnali
void *threadSignalBody(void *args);


// Funzione che crea i thread per il calcolo del cammino minimo dall'
// intero a all'intero b
void camminiMinimiThreadCreate(int fd, atomic_int* stato_terminazione, attore *arrayGrafo, int capacity,pthread_t *signalThread);


// Funzione per il calcolo del cammino minimo tra a e b (eseguito da un therad)
// implementa la BFS
void *threadMinPathBody(void *args);


// Funzione per la deallocazione dovuta all'arrivo di segnale SIGINT
void signalDeallocation(attore *arrayGrafo, int capacity, pthread_t *signalThread);


// Funzione che crea l'ABR
nodoABR* creaABR(attore *actor, nodoABR *predec);


// Funzione che inserisce nell'ABR dei visitati l'ABR appena creato di
// un coprotagonista di un nodo estratto dalla testa della FIFO
void inseriscoInABR(nodoABR **radice, nodoABR *nodo);


// Funzione che dealloca l'ABR
void deallocaABR(nodoABR* radice);


// Funzione per l'inserimento in testa a FIFO di un attore
void inserimentoInCoda(FIFO *queue, attore *att);


// Funzione che implementa l'estrazione dalla testa
attore *estrazioneInTesta(FIFO *fifo);


// Funzione che cerca nell'ABR il codice dell'attore estratto in testa dalla FIFO
nodoABR *cercaInABR(nodoABR *radice, int codiceShuffle);


// Funzione per deallocare la FIFO creata
void deallocoFIFO(FIFO *fifo);


// Funzione che calcola il tempo che impiega il programma a trobare un cammino
// minimo tra a ed b
double calcolaTempo(clock_t inizio, clock_t fine);


// Funzione che stampa su file o su stdout
void stampaCamminoMinimo(int a, int b, double tempo, nodoABR *destinazione, int controlValue);


int shuffle(int n);


int unshuffle(int n);