# Grafo-degli-Attori

Descrizione delle specifiche richieste dal testo del progetto completo:
1. il dettaglio di come avviene il parsing delle righe del file di testo name.basics.tsv nel programma Java
2. il dettaglio dell'implementazione della coda FIFO nell'algoritmo BFS, e quali informazioni vengono memorizzate in ogni elemento della coda
3. il dettaglio di come vengono ricostruiti i nodi intermedi del cammino minimo in cammini.c
4. il dettaglio di come il thread gestore di segnali comunica al programma principale di interrompere l'elaborazione
5. per il progetto completo: in quale modo vengono memorizzate e gestite le partecipazioni degli attori all'interno del programma Java

## CreaGrafo.java

1) Parsing delle righe del file name.basics.tsv: all'interno del metodo main della classe CreaGrafo.java chiamato il metodo LeggiDaFileNomi che prende per argomento argv[0] ovvero il file name.basics.tsv e una mappa chiave valore, inizialmente vuota, dove la chiave è un Integer e il valore è un'istanza della classe Attore.
LeggiDaFileNomi leggere singolarmente ogni riga del file name.basics.tsv. Per ogni riga del file estrae i singoli campi e li inserisce in un array in modo da essere più facilmente gestibili. Vengono verificati i vincoli che erano stati richiesti dal progetto.
Se superano tutti i controlli allora viene creata una nuova istanza di attore associata al codice univoco trovato come primo argomento del file ed inserita nella mappa.
Questo processo è gestito da un while che legge le linee fino a che non arriva in fondo al file.

5) Memorizzazione e gestione delle partecipazioni: per la memorizzazione e la gestione delle partecipazioni si utilizza sempre la mappa chiave valore dove la chiave è il codice univoco identificativo dell'attore mentre il valore associato alla chiave è l'istanza della classe Attore che possiede tale  codice.
Grazie al metodo ScriviInPartecipazioni viene creata una lista dei codici degli attori presenti nella mappa. Grazie ad un for, per ogni codice identificatore degli attori creo la lista dei codici identificativi dei film ,cui l'attore preso in considerazione, a cui ha preso parte. Questo è possibile farlo perché nella classe Attore è presente un Set per i codici univoci degli attori che hanno lavorato con l'attore preso un considerazione e un Set per l'insieme dei codici dei film cui l'attore in questione ha lavorato.
Successivamente scrive nel file il codice identificativo dell'attore il numero dei film cui ha preso parte e tutti i codici di tali film.

cammini.c

2) Implementazione della coda FIFO: la coda FIFO viene implementata all'interno di threadMinPathBody, che implementa la BFS, come un array di puntatori ad attori. All'interno di functionality.h è presente una struct per l'implementazione della FIFO che memorizza per ogni elemento della coda: codiceTesta e codiceCoda che sono due interi per l'estrazione e l'inserimento all'interno della coda, capacity che rappresenta la capacità dell'array ed infine coda che è l'array di puntatori ad attori.

3) Ricostruzione dei nodi intermedi de cammino minimo: la ricostruzione dei nodi intermedi del cammino minimo vengono ricostruiti grazie al fatto che ogni nodo (struttura nodoABR) dell'ABR presenta il puntatore al nodo predecessore, cioè ogni volta che vengono scoperti nodi adiacenti viene settato il predecessore. In questo modo non appena viene trovata la destinazione viene chiamata la funzione stampaCamminoMinimo cui prende, tra tutti i parametri, anche il puntatore alla destinazione. In tal modo viene creato un array di attori, partendo dalla destinazione si risale fino al nodo sorgente (ovviamente passo passo si carica il puntatore alla struttura attore di ogni nodo nell'array). A questo punto viene fatto scorrere l'array in senso inverso in modo tale da poter stampare all'interno del file il cammino minimo nel verso corretto

4) Gestione interruzione elaborazione: all'inizio del main viene creato ed avviato un thread gestori di segnali SIGINT. Tale thread quando viene avviato gli viene passata la struttura datiThreadSignal che contiene due flags (variabili di tipo "atomic_int" condivise con il main) su cui si basa il comportamento della funzione passata al thread al momento della creazione ovvero threadSignalBody. La struttura detiThreadSignal presenta attiva_lettura_pipe, puntatore condiviso con il main che rappresenta lo stato della pipe cioè se è stata o meno creata (se è stata creata si termina nel modo in cui è descritto nelle specifiche, altrimenti si stampa un messaggio di attesa), questo avviene nel main non appena viene creata la pipe. Inoltre è presente anche stato_terminazione, anche lui puntatore condiviso con il main che viene settato solo in caso di arrivo di un SIGINT dopo il cambio di stato della pipe.
Nel caso in cui arriva un SIGINT dopo il cambio di stato della pipe, stato_terminazione deve comunicare al main di terminare. stato_terminazione viene quindi settato ad 1 e termina, nel mentre il main terminerà anchesso nella modalità richiesta.
Nel caso in cui invece non si riceva SIGINT e si procede quindi alla terminazione naturale del programma, per fa terminare il thread e per poterlo joinarlo il thread main gli invia un SIGINT tramite pthread_kill() che porta il thread a terminare per poi essere joinato e terminare definitivamente il programma.
