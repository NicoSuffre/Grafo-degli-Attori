#include "functionality.h"

// Funzione di confronto per la bsearch
int confrontaCodici(const void * codice1, const void *codice2){

	int *cod1 = (int *) codice1;
	attore *cod2 = (attore *) codice2;

	if(*cod1 < cod2->codice)
	{
		return -1;
	}

	if(*cod1 > cod2->codice)
	{
		return 1;
	}

	return 0;
}

attore* inserimentoAttori(FILE *nomeFile, int *capacity){
	
	// Controllo se il punatore FILE è null
	assert(nomeFile != NULL);	

	// Controllo sul valore della capacità dell'array
	assert(*capacity == 0);

	// Dimensione del buffer per allocare attori poco alla volta
	int dimBufArray = 10000;

	attore *arrGraph = malloc(dimBufArray * sizeof(attore));
	if(arrGraph == NULL)
	{
		xtermina("Errore nella malloc\n",QUI);
	}

	// Inizializzo dichiarazione (ed inizializzazione per nomeAttore)
	// delle variabili di appoggio per l'insirimento dei valori letti da file
	int codiceAttore;
	int annoAttore;
	char *nomeAttore = NULL;

	// Variabili per utilizzo della getline()
	char *buffer = NULL;
	size_t n = 0;

	while(getline(&buffer, &n, nomeFile) > 0){

		char *puntatoreInterno;

		// Leggo valori da una linea
		codiceAttore = atoi(strtok_r(buffer,"\t",&puntatoreInterno));
		nomeAttore = strtok_r(NULL,"\t",&puntatoreInterno);
		annoAttore = atoi(strtok_r(NULL,"\t",&puntatoreInterno));

		// Controllo la dimensione dell'array di attori
		if(*capacity == dimBufArray)
		{
			// Aumento la dimensione dell'array di attori
			dimBufArray *= 2;
			
			// Alloco lo spazio necessario
			arrGraph = realloc(arrGraph, dimBufArray * sizeof(attore));
		}

		// Inserisco i dati nell'attore
		arrGraph[*capacity].codice = codiceAttore;
		arrGraph[*capacity].nome = strdup(nomeAttore);
		arrGraph[*capacity].anno = annoAttore;

		// Incremento il valore dell'indice
		(*capacity)++;
	}

	// Controllo tra indice e dimBufArray
	assert(*capacity <= dimBufArray);

	// Realloco la memoria dell'array di attori in modo tale
	// da non sprecarne
	arrGraph = realloc(arrGraph, *capacity * sizeof(attore));

	// Dealloco il buffer per getline
	free(buffer);

	return arrGraph;
}


void completamentoInserimentoAttori(FILE *nomeFile, attore *arrayGrafo,int *capacity, int numeroConsumer){
	// Indice di inserimento nel buffer (da passare al producer)
	int prodIndex = 0;

	// Indice di estrazione dal buffer (da passare ai consumers)
	int consIndex = 0;

	// Dichiarazione del buffer
	char* buffer[buffSize];

	// Dichiarazione del mutex per i consumers
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	// Inizializzazione del mutex
	xpthread_mutex_init(&mutex, NULL, QUI);

	// Semaforo per il numero di slots vuoti nel buffer
	sem_t sem_free_slots;

	// Semaforo per il numero di posti occupati nel buffer
	sem_t sem_data_items;

	// Inizializzazione sempafori
	xsem_init(&sem_free_slots,0,buffSize,QUI);
	xsem_init(&sem_data_items,0,0,QUI);

	// Dichiarazione del thread Producer
	datiProducer produttore;

	// Definizione array di consumers
	pthread_t arrThreadConsumer[numeroConsumer];

	// Dichiarazione array dei dati da passare ai consumers
	datiConsumer arrDatiConsumer[numeroConsumer];


	fprintf(stderr,"==== INIZIALIZZAZIONE E PARTENZA DEI THERADS CONSUMERS ====\n");
	
	// Creazione dei thread, e li faccio partire
	for(int i = 0; i < numeroConsumer; i++){
		arrDatiConsumer[i].buffer = buffer;
		arrDatiConsumer[i].cIndex = &consIndex;
		arrDatiConsumer[i].mutex = &mutex;
		arrDatiConsumer[i].sem_free_slots = &sem_free_slots;
		arrDatiConsumer[i].sem_data_items = &sem_data_items;
		arrDatiConsumer[i].arrayGrafo = arrayGrafo;
		arrDatiConsumer[i].capacity = *capacity;
		
		// Avvio il thread i-esimo
		xpthread_create(&arrThreadConsumer[i],NULL,threadConsBody,&arrDatiConsumer[i],QUI);
	}

	// Inizializzazione delle variabili del Producer
	produttore.buffer = buffer;
	produttore.pIndex = &prodIndex;
	produttore.sem_free_slots = &sem_free_slots;
	produttore.sem_data_items = &sem_data_items;
	produttore.nomeFile = nomeFile;


	fprintf(stderr,"==== AVVIO LETTURA TRAMITE PRODUCER ====\n");

	threadProdBody(&produttore);

	fprintf(stderr,"==== FINE LETTURA TRAMITE PRODUCER ====\n");

	// Terminazione dei threads Consumers
	for(int i = 0; i < numeroConsumer; i++){
		xsem_wait(&sem_free_slots,QUI);
		buffer[prodIndex] = NULL;
		xsem_post(&sem_data_items,QUI);
		prodIndex = (prodIndex + 1) % buffSize;
	}

	// Join dei threads
	for(int i = 0; i < numeroConsumer; i++){
		xpthread_join(arrThreadConsumer[i],NULL,QUI);
	}

	// Distruzione del mutex
	xpthread_mutex_destroy(&mutex,QUI);

	// Distruzione dei semafori
	xsem_destroy(&sem_free_slots,QUI);
	xsem_destroy(&sem_data_items,QUI);
}


void *threadProdBody(void *args){
	if(args == NULL)
	{
		xtermina("ERRORE: argomenti passati al corpo del Producer non validi (NULL)\n",QUI);
	}

	datiProducer *datiP = (datiProducer* )args;

	// Variabili per l'utilizzo di getline()
	char *bufferLine = NULL;
	size_t n = 0;

	while(getline(&bufferLine, &n, datiP->nomeFile) > 0){
		// controllo bufferline == NULL?

		xsem_wait(datiP->sem_free_slots,QUI);
		
		// Carico nel buffer condiviso alla posizione pIndex la linea letta dal file grafo.txt
		datiP->buffer[*datiP->pIndex] = strdup(bufferLine);
		
		// Aggiorno il valore dell'indice di inserimento del produttore
		*datiP->pIndex = ((*datiP->pIndex) + 1) % buffSize;

		xsem_post(datiP->sem_data_items,QUI);
	}

	// Dealloco il bufferLine di getline()
	free(bufferLine);

	return NULL;
}


void *threadConsBody(void *args){

	if(args == NULL)
	{
		xtermina("ERRORE: argomanti passati al corpo dei Consumers non validi (NULL)\n",QUI);
	}

	datiConsumer *datiC = (datiConsumer* )args;

	while(true){

		// Aspetto che ci sia qualcosa nel buffer
		xsem_wait(datiC->sem_data_items,QUI);

		// Lock del mutex
		xpthread_mutex_lock(datiC->mutex,QUI);

		char *linea = datiC->buffer[*(datiC->cIndex)];
		*(datiC->cIndex) = (*(datiC->cIndex) + 1) % buffSize;

		if(linea == NULL)
		{
			xpthread_mutex_unlock(datiC->mutex,QUI);
			xsem_post(datiC->sem_free_slots,QUI);
			break;
		}
		
		xpthread_mutex_unlock(datiC->mutex,QUI);
		xsem_post(datiC->sem_free_slots,QUI);

		char *puntatoreInterno;

		int codAtt = atoi(strtok_r(linea,"\t",&puntatoreInterno));
		int numeroCoprotag = atoi(strtok_r(NULL,"\t",&puntatoreInterno));

		int *coprotagonisti = NULL;

		if(numeroCoprotag > 0)
		{
			coprotagonisti = malloc(numeroCoprotag * sizeof(int));
			
			// Controllo se malloc di coprotagonisti è andato a buin fine
			if(coprotagonisti == NULL)
			{
				free(linea);
				xtermina("ERRORE: malloc di array coprotagonisti in threadConsBody è pari a NULL\n",QUI);
			}

			for(int i = 0; i < numeroCoprotag; i++){
				coprotagonisti[i] = atoi(strtok_r(NULL,"\t",&puntatoreInterno));
			}
		}

		// Dealloco il buffer per la getline()
		free(linea);

		attore *attoreAppoggio = bsearch(&codAtt, datiC->arrayGrafo, datiC->capacity, sizeof(attore), confrontaCodici);

		assert(attoreAppoggio != NULL);

		attoreAppoggio->numcop = numeroCoprotag;
		attoreAppoggio->cop = coprotagonisti;
	}

	// Termino i thread
	pthread_exit(NULL);
}


void *threadSignalBody(void *args){

	if(args == NULL)
	{
		xtermina("ERRORE: argomanti passati al corpo del Thread Gestore dei Segnali non validi (NULL)\n",QUI);
	}

	// Stampo il pid del thread gestore ddi segnali
	fprintf(stdout,"Thread Gestore Signal: Il mio pid e': %d\n",getpid());
	
	datiThreadSignal *datiTS = (datiThreadSignal* )args;

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGINT);

	// Valore di ritorno della sigwait()
	int err;

	// Numero di segnale che verrà catturato dalla sigwait
	int numS;

	while(atomic_load(datiTS->stato_terminazione) == 0){
		err = sigwait(&mask,&numS);
		
		// Se err == 0 ho ricevuto il segnale correttamente, se != da 0
		// c'è stato un problema
		if(err != 0)
		{
			xtermina("ERRORE: errore nelle sigwait (threadSignalBody)\n",QUI);
			break;
		}

		// Se il valore di attiva_lettura_pipe è 0 vuol dire che non
		// sono ancora in fase di lettura della pipe, mentre se è 1
		// allora sono in fase di lettura della pipe e quindi il programma
		// deve terminare come nelle specifiche
		if(atomic_load(datiTS->attiva_lettura_pipe) == 1)
		{
			// Siccome la pipe è aperta allora posso dire al programma di terminare
			atomic_store(datiTS->stato_terminazione, 1);
		}
		else
		{
			fprintf(stdout,"Costruzione del grafo in corso\n");
		}
	}
	return NULL;
}


void *threadMinPathBody(void *args){

	if(args == NULL)
	{
		xtermina("ERRORE: argomenti passati al thread del calcolo del cammino minimo non validi (NULL)\n",QUI);
	}

	datiThreadMinPath *datiTMP = (datiThreadMinPath* )args;

	// Inizio la misurazione del tempo di esecuzione
	clock_t start = times(NULL);

	// Ottenngo dall'array di attpri gli attori con i codici cercati
	attore *sorgente = bsearch(&datiTMP->a, datiTMP->arrayGrafo, datiTMP->capacity, sizeof(attore), confrontaCodici);
	attore *destinazione = bsearch(&datiTMP->b, datiTMP->arrayGrafo, datiTMP->capacity, sizeof(attore), confrontaCodici);
	
	// Controllo che i codici che mi sono stati dati siano validi
	if(sorgente == NULL || destinazione == NULL)
	{
		// Uno dei due codici non è valido
		// Stoppo il timer e calcolo qunato tempo è passato
		//double tempo = calcolaTempo(start, time(NULL));
		double tempo = calcolaTempo(start, times(NULL));

		int valoreStampa;
		if(sorgente == NULL)
		{
			valoreStampa = -1;
		}
		else
		{
			valoreStampa = 0;
		}

		stampaCamminoMinimo(datiTMP->a, datiTMP->b, tempo, NULL, valoreStampa);

		// Dealloco gli argomenti passati (struct dei dati del thread)
		free(datiTMP);
		return NULL;
	}

	nodoABR *visitati = creaABR(sorgente,NULL);
	assert(visitati != NULL);

	// Controllo se la sorgente e la destinazione sono uguali
	if(datiTMP->a == datiTMP->b)
	{
		// Stoppo il timer e calcolo qunato tempo è passato
		double tempo = calcolaTempo(start, times(NULL));

		stampaCamminoMinimo(datiTMP->a, datiTMP->b, tempo, visitati, 1);

		// Dealloco l'abr (unico nodo)
		deallocaABR(visitati);
		
		// Dealloco gli argomenti passati (struct dei dati del thread)
		free(datiTMP);
		return NULL;
	}

	// Dichiaro ed inizializzo la FIFO
	FIFO raggiunti;
	raggiunti.capacity = 1024;
	raggiunti.codiceTesta = 0;
	raggiunti.codiceCoda = 0;
	raggiunti.coda = malloc(raggiunti.capacity * sizeof(attore));

	// Controllo per la malloc
	if(raggiunti.coda == NULL)
	{
		perror("Errore malloc");
		fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(),QUI);
		exit(1);
	}

	// Inserisco la sotgente in FIFO
	inserimentoInCoda(&raggiunti, sorgente);

	while(raggiunti.codiceTesta < raggiunti.codiceCoda){
		
		// Estraggo la testa da FIFO
		attore *attEstratto = estrazioneInTesta(&raggiunti);

		assert(attEstratto != NULL);

		nodoABR *attEstrattoABR = cercaInABR(visitati, shuffle(attEstratto->codice));
		assert(attEstrattoABR != NULL);

		// Inserisco i nodi adiacenti
		for(int i = 0; i < attEstratto->numcop; i++){

			// Controllo se ho già visitato l'i-esimo nodo adiacente
			if(cercaInABR(visitati, shuffle(attEstratto->cop[i])) != NULL)
			{
				continue;
			}

			// In caso negativo, ottengo l'attore e aggiorno l'ABR
			attore *adiacente = bsearch(&attEstratto->cop[i], datiTMP->arrayGrafo, datiTMP->capacity, sizeof(attore), confrontaCodici);
			assert(adiacente != NULL);

			nodoABR *adiacenteABR = creaABR(adiacente, attEstrattoABR);

			// Inserisco in visitti l'ABR di "adiacente" appena trovato
			inseriscoInABR(&visitati, adiacenteABR);

			// Controllo se il codice dell'attore adiacente sia uguale alla destinazione
			if(adiacente->codice == datiTMP->b)
			{
				// Ho trovato la destinazione
				// Stoppo il timer e calcolo quanto tempo è passato
				double tempo = calcolaTempo(start, time(NULL));

				stampaCamminoMinimo(datiTMP->a, datiTMP->b, tempo, adiacenteABR, 1);
				
				// Dealloco ABR creato
				deallocaABR(visitati);

				// Dealloco la FIFO
				deallocoFIFO(&raggiunti);

				// Dealloco gli argomenti passati (struct dei dati del thread)
				free(datiTMP);

				return NULL;
			}

			// Altrimenti aggiungo alla FIFO
			inserimentoInCoda(&raggiunti,adiacente);
		}
	}

	// A questo punto è stata eseguita la BFS senza però trovare un cammino da a ad b

	// Stoppo il timer e calcolo qunato tempo è passato
	double tempo = calcolaTempo(start, times(NULL));

	stampaCamminoMinimo(datiTMP->a, datiTMP->b, tempo, NULL, 0);

	// Dealloco l'ABR creato
	deallocaABR(visitati);

	// Dealloco la FIFO
	deallocoFIFO(&raggiunti);

	// Dealloco gli argomenti passati (struct dei dati del thread)
	free(datiTMP);

	return NULL;
}


void camminiMinimiThreadCreate(int fd, atomic_int* stato_terminazione, attore *arrayGrafo, int capacity,pthread_t *signalThread){

	fprintf(stderr,"==== INIZIO LETTURA DALLA PIPE ====\n");

	// Creo un buffer per la lettura della pipe, di dimensione 2 (Allocato staticamente
	// non dinamicamente)
	int32_t pipeBuffer[2];

	// Variabile che ospiterà il valore di ritorno della read, ovvero il
	// numero di byte letti dalla pipe
	ssize_t byteLetti;

	fprintf(stderr,"= sono prima del while di camminiMinimiThreadCreate =\n");

	// Ciclo while che termina solo quando stato_terminazione == 1, quando non lo è
	// crea per ogni coppia di interi a 32 bit un thread per il calcolo del cammino minimo
	while (atomic_load(stato_terminazione) != 1) {
		
		fprintf(stderr,"= SONO DENTRO AL WHILE DI camminiMinimiThreadCreate =\n");

		// lettura dalla pipe, ritorna -1 se ci sono stati problemi altrimenti
		// ritorna il numero di byte letti
		byteLetti = read(fd,pipeBuffer, 2 * sizeof(int32_t));

		// Se byteLetti == 0 vuol dire che lo scrittore
		// ha chiuso la sua estremità della pipe
		if(byteLetti == 0)
		{	
			// Esco dal while per la lettura della pipe
			break;
		}

		// Se inevece è minore di 0 byte in qualunque caso c'è stato un errore
		// nella lettura della pipe
		if(byteLetti < 0)
		{
			xtermina("ERRORE: errore in lettura dalla pipe\n",QUI);
		}

		// Se inevece byteLetti è < 8, allora c'è stata una lettura
		// parziale dei dati
		if(byteLetti < 8)
		{
			xtermina("ERRORE: errore di lettura dei dati dalla pipe\n",QUI);
		}

		int32_t a = pipeBuffer[0];
		int32_t b = pipeBuffer[1];

		// Alloco lo spazio necessario per la struct dei dati da passare
		// ai thread che calcolano il cammino minimo
		datiThreadMinPath *datiTMP = malloc(sizeof(datiThreadMinPath));

		// Controlli sulla corretta aloocazione della strict
		if(datiTMP == NULL)
		{
			xtermina("ERRORE: errore nell'allocazione della struct per il therad che calcola il cammino minimo", QUI);
		}

		// Inserisco i dati all'interno della struct
		datiTMP->a = a;
		datiTMP->b = b;
		datiTMP->arrayGrafo = arrayGrafo;
		datiTMP->capacity = capacity;

		pthread_t threadMP;
		xpthread_create(&threadMP, NULL, threadMinPathBody, datiTMP,QUI);

		// Rendo detach il thread
		int detachControllo = pthread_detach(threadMP);
		if(detachControllo != 0)
		{
			perror("ERRORE: errore in pthread_detach");
			fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),QUI);
			pthread_exit(&threadMP);
		}
	}

	fprintf(stderr,"==== FINE LETTURA DELLA PIPE ====\n");

	// Chiudo la pipe
	xclose(fd,QUI);

	// Elimino la pipe
	int pipeControllo = unlink("./cammini.pipe");
	if(pipeControllo != 0)
	{
		xtermina("ERRORE: errore nell'eliminazione della pipe\n",QUI);
	}

	// Controllo se stato_terminazione == 1 questo perchè non so se sono
	// uscito dal while perchè la pipe è stata chiusa oppure perchè è 
	// arrivato un segnale SIGINT
	if(atomic_load(stato_terminazione) == 1)
	{
		fprintf(stderr,"=sto per stampare il messaggio di terminazione dovuto al segnale nell'if finele di camminiMinimiThreadCreate =\n");
		fprintf(stderr,"== TERMINAZIONE DOVUTA A SEGNALE SIGINT ==\n");

		// Dealloco le risorse del occupate dal grafo degli attori e
		// faccio la join del therad gestore di seganli
		signalDeallocation(arrayGrafo,capacity,signalThread);

		// Termino il programma tramite exit() con codice EXIT_FAILURE
		exit(EXIT_FAILURE);
	}
}


nodoABR *creaABR(attore* att, nodoABR *predec){

	nodoABR *nodo = malloc(sizeof(nodoABR));
	if(nodo == NULL)
	{
		perror("Errore malloc");
		fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(),QUI);
		exit(1);
	}

	nodo->actor = att;
	nodo->codActShuffle = shuffle(att->codice);
	nodo->predecessore = predec;
	nodo->figlioDx = NULL;
	nodo->figlioSx = NULL;

	return nodo;
}


void inseriscoInABR(nodoABR **radice, nodoABR *nodo){

	assert(nodo != NULL);
	if(*radice == NULL)
	{
		*radice = nodo;
		return;
	}

	if((*radice)->codActShuffle == nodo->codActShuffle)
	{
		// Nodo già presente allora si scarta
		free(nodo);
		return;
	}

	// Controllo se inserire il nodo a nel ramo destro o sinistro
	if((*radice)->codActShuffle > nodo->codActShuffle)
	{
		return inseriscoInABR(&(*radice)->figlioSx,nodo);
	}
	else
	{
		return inseriscoInABR(&(*radice)->figlioDx, nodo);
	}
}


void deallocaABR(nodoABR *radice){

	if(radice != NULL)
	{
		deallocaABR(radice->figlioDx);
		deallocaABR(radice->figlioSx);
		free(radice);
	}
}


void inserimentoInCoda(FIFO *fifo, attore *att){

	assert(fifo != NULL);

	// Controllo se la 
	if(fifo->codiceCoda >= fifo->capacity)
	{
		if(fifo->codiceCoda > 32)
		{
			int dim = fifo->codiceCoda - fifo->codiceTesta;

			memmove(fifo->coda, fifo->coda + fifo->codiceTesta, dim * sizeof(attore*));

			fifo->codiceCoda = dim;
			fifo->codiceTesta = 0;
		}
		else
		{
			fifo->capacity *= 2;
			fifo->coda = realloc(fifo->coda, fifo->capacity * sizeof(attore*));

			// Controllo realloc
			if(fifo->coda == NULL)
			{
				perror("Errore remalloc");
				fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(),QUI);
				exit(1);
			}
		}
	}

	fifo->coda[fifo->codiceCoda] = att;
	fifo->codiceCoda++;
}


attore *estrazioneInTesta(FIFO *fifo){

	assert(fifo != NULL);

	assert(fifo->codiceCoda != fifo->codiceTesta);

	return fifo->coda[(fifo->codiceTesta++)];
}


nodoABR *cercaInABR(nodoABR *radice, int codiceShuffle){

	if(radice == NULL)
	{
		return NULL;
	}

	if(radice->codActShuffle == codiceShuffle)
	{
		return radice;
	}

	if(radice->codActShuffle > codiceShuffle)
	{
		return cercaInABR(radice->figlioSx, codiceShuffle);
	}
	else
	{
		return cercaInABR(radice->figlioDx, codiceShuffle);
	}
}


void signalDeallocation(attore *arrayGrafo, int capacity, pthread_t *signalThread){

	// join del thread gestore di sehnali
	xpthread_join(*signalThread, NULL, QUI);

	// Attendo 20 secondi
	sleep(20);

	// Dealloco le risorse all'interno dell'array di attori
	for(int i = 0; i < capacity; i++){
		free(arrayGrafo[i].nome);
		free(arrayGrafo[i].cop);
	}

	free(arrayGrafo);
}


void deallocoFIFO(FIFO *fifo){

	free(fifo->coda);
}


double calcolaTempo(clock_t inizio, clock_t fine){

	// Calcolo il numero di clock tick trascorsi
	//clock_t clock_trascorsi = fine - inizio;

	// Converto tutto secondi  
	double tempoInSecondi = (double)(fine - inizio) / sysconf(_SC_CLK_TCK);

	return tempoInSecondi;
}

void stampaCamminoMinimo(int a, int b, double tempo, nodoABR *destinazione, int controlValue){

	// Dichiaro un buffer per il nome del file
	char bufferNomeF[32];

	// Creo il nome del file
	snprintf(bufferNomeF,sizeof(bufferNomeF),"%d.%d",a,b);

	// Apro il file dedicato alla stampa
	FILE *fileStampa = xfopen(bufferNomeF, "w",QUI);

	if(controlValue == 1)
	{
		// Andato tutto bene
		int lunghezza = 0;
		int capacity = 32;

		attore *path = malloc(capacity * sizeof(attore));
		if(path == NULL)
		{
			perror("Errore malloc");
			fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(),QUI);
			exit(1);
		}

		while(destinazione != NULL){
			if(lunghezza == capacity)
			{
				capacity *= 2;
				path = realloc(path,capacity * sizeof(attore));
				if(path == NULL)
				{
					perror("Errore realloc");
					fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(),QUI);
					exit(1);
				} 
			}
			path[lunghezza++] = *destinazione->actor;
			destinazione = destinazione->predecessore;
		}

		// Scorro all'indietro l'array di attori
		for(int i = lunghezza - 1; i >= 0; i--){
			fprintf(fileStampa, "%d\t%s\t%d\n", path[i].codice, path[i].nome, path[i].anno);
		}

		// Dealloco il path
		free(path);
		fprintf(stdout,"%s: Lunghezza minima %d. Tempo di elaborazione %.3f secondi\n",bufferNomeF,lunghezza-1,tempo);
 	}else{
 		// Ottenuto un esito negativo
 		fprintf(stdout,"%s: Nessun cammino. Tempo di elaborazione %.3f secondi\n",bufferNomeF,tempo);

 		// Se non ci sono cammini tra a e b
 		if(controlValue == 0)
 		{
 			fprintf(fileStampa,"non esistono cammini da %d a %d\n",a,b);
 		}
 		else
 		{
 			// Nessun cammino perchè usorgente non è valida
 			fprintf(fileStampa,"codice %d non valido\n",a);
 		}
 	}

 	if(fclose(fileStampa) == EOF)
 	{
 		xtermina("ERRORE: errore chiusura file path",QUI);
 	}

 	return;
}


int shuffle(int n){
  return ((((n & 0x3F) << 26) | ((n >> 6) & 0x3FFFFFF)) ^ 0x55555555);
}


int unshuffle(int n){
  return ((((n >> 26) & 0x3F) | ((n & 0x3FFFFFF) << 6)) ^ 0x55555555);
}