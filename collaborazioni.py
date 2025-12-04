#!/usr/bin/env python3

import sys

# Funzione che prende per argomento il file partecipazioni, lo apre
# in lettura e crea un dizionario in cui associa ad ogni chiave, cioè
# ad ogni codice di un attore, un Set di interi che sono i codici unioci
# dei film in cui ha lavorato
def letturaPartecipazioni(filePartecipazioni):
	partecipazioni = {}

	with open(filePartecipazioni,'r') as file:
		for line in file:

			# Crea una lista di stringhe "campi = ["str1","str2",..]", partendo
			# dalla linea del file il metodo strip() elimina spazi pbianchi all'inizio
			# e alla fine della riga, elimina anche \n e \t se si trovano all'inizio o 
			# alla fine, mentre split separa le parole della linea se sono divise da '\t'
			campi = line.strip().split('\t');

			codAtt = int(campi[0])
			numFilm = int(campi[1])

			# Creo un Set per i codici dei film
			setFilm = set()

			# Scorro gli elementi della lista campi a partire dal campo 2 fino a che non
			# arrivo in fondo alla lista ovvero numero di film + 2  
			for i in range(2, 2 + numFilm):
				setFilm.add(int(campi[i]));

			# Inserisco nel Dizionario la coppia <chiave,valore>: codAtt, setFilm
			partecipazioni[codAtt] = setFilm

	# Restituisco il Dizionario creato
	return partecipazioni

# Funzione che apre in lettura il file title.basics.tsv e crea un diszionario in cui
# la chiave il codice univo del film e il valore è il titolo del film
def letturaTitle(fileTitle):
	titoli = {}

	with open(fileTitle, 'r') as file:

		# Salto la riga di intestazione
		file.readline()

		for line in file:

			campi = line.strip().split('\t')

			# prendo il codice che è di queto tipo: tt00000001
			codiceAppoggio = campi[0]

			# trasformo in intero il codice escludento il 'tt'
			codiceFilm = int(codiceAppoggio[2:])

			titolo = campi[2]

			# Creo il diszionario con chiave il codice del film e come valore
			# il titolo del film
			titoli[codiceFilm] = titolo

	# Ritorno il dizionario
	return titoli


# Fuunzione che cerca i film in cui actor1 e actor2 hanno lavorato insieme,
# se viene trovata una corrispondenza restituisce una lista di tuple (filmCodice, titoloFilm)
def cercaFilmCollaborazione(actor1, actor2, partecipazioni, titles):

	# Controllo l'esistenza dei due attori in "partecipazioni"
	if actor1 not in partecipazioni or actor2 not in partecipazioni:
		return []

	# Faccio intersezione dei Set dei film dei due attori
	filmInComune = partecipazioni[actor1] & partecipazioni[actor2]

	filmInCollaborazione = []
	for actorCode in filmInComune:
		titolo = titles.get(actorCode, "< Titolo assente >")
		filmInCollaborazione.append((actorCode,titolo))

	# Ordino in maniera crescente i valori dei codici degli attori
	filmInCollaborazione.sort()

	return filmInCollaborazione
		

def main():

	# Controllo sul numero di argomenti passati da linea di comando
	if(len(sys.argv) < 5):
		print(f"Uso:\n\t {sys.argv[0]} partecipazioni.txt titile.basics.tsv codice1 codice2 [codice3 codice4 ...]")
		sys.exit(1)

	# Stampa del comando passato da linea di comando
	print("$", *sys.argv)

	filePartecipazioni = sys.argv[1]
	fileTitle = sys.argv[2]

	# Lista in cui inserisco i codici degli attori passati da linea di comando
	codiciAttori = []

	# Ciclo for che scorre tutti gli elmenti, dal terzo in poi, passati da linea di comando
	for cod in sys.argv[3:]:
		
		# Cast da stringa ad intero
		codice = int(cod)
		codiciAttori.append(codice)

	#print("=== INIZIO LETTURA FILE partecipazioni.txt ===\n",sys.stderr);
	partecipazioni = letturaPartecipazioni(filePartecipazioni)
	#print("=== FINE LETTURA FILE partecipazioni.txt ===\n",sys.stderr);

	#print("=== INIZIO LETTURA FILE title.basics.tsv ===\n",sys.stderr);
	titles = letturaTitle(fileTitle)
	#print("=== FINE LETTURA FILE title.basics.tsv ===\n",sys.stderr);

	#print("=== INIZIO RICERCA DEI FILM PRODOTTI IN COLLABORAZIONE PER OGNI ATTORE AVUTO DA LINEA DI COMANDO ===\n",sys.stderr);

	for i in range(len(codiciAttori) - 1):

		actor1 = codiciAttori[i]
		actor2 = codiciAttori[i + 1]

		filmInCollaborazione = cercaFilmCollaborazione(actor1, actor2, partecipazioni, titles)

		numCollaborazioni = len(filmInCollaborazione)

		if numCollaborazioni == 0:
			print(f"{actor1}.{actor2} nessuna collaborazione")
		else:
			print(f"{actor1}.{actor2}: {numCollaborazioni} colaborazioni:")
			for filmCode, filmTitle in filmInCollaborazione:
				print(f" {filmCode:7d} {filmTitle}")

		# Inserisco una linea vuota tra ogni coppia di attori
		print()

	print("=== Fine")


if __name__ == '__main__':
	main()