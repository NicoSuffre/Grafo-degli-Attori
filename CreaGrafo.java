import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.util.HashMap;
import java.util.Map;
import java.io.FileWriter;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;


public class CreaGrafo {
    
    private static void LeggiDaFileNomi(String nomefile, Map<Integer,Attore> attori){
            try{
                BufferedReader bufRead = new BufferedReader(new FileReader(nomefile));
                
                // Salto l'intestazione (nconst primaryName birthYear deathYear primaryProfession knownForTitles)
                bufRead.readLine();

                // Stringa cui verra assegnata una linea del file alla volta
                String singolaLinea;
                while((singolaLinea = bufRead.readLine()) != null){
                    
                    // Ottengo un array dove ogni singolo elemento è un campo dato dal file
                    // (es: all'indice 0 avrò nconst, all'indice 1 avrò primaryName e così via
                    //  fino al campo 5 dove avrò knownForTitles)
                    String[] CampiSingolaLinea = singolaLinea.split("\t");
                    
                    // Se il campo 2 (cioè birthYear) salto alla linea successiva
                    if(CampiSingolaLinea[2].equals("\\N"))
                    {
                        continue;
                    }
                    
                    // Se il campo 4 (cioè primaryProfession) non contiene nè "actor" nè "actress" salto alla linea successiva
                    if(CampiSingolaLinea[4].contains("actor") == false)
                    {
                        if(CampiSingolaLinea[4].contains("actress") == false)
                        {
                            continue;
                        }
                    }
                    
                    // Assegno a "codiceUnivoco" la stringa corrispondende al campo zero (cioè nconst)
                    // senza i primi due caratteri (ovvero nm) grazie al metodo "substring(2)" e poi
                    // la converte in un numero intero grazie al metodo "parseInt"
                    int codiceUnivoco = Integer.parseInt(CampiSingolaLinea[0].substring(2));
                    
                    // Assegno a "nome" il nome presente nel campo 1 (cioè primaryName)
                    String nome = CampiSingolaLinea[1];

                    // Assegno a "dataNascita" il valore della data di nascita pressente nel campo 2
                    // (cioè birthYear) ovviamente precedentmente trasformato in intero tramite il
                    // metodo "parseInt()"
                    int dataNascita = Integer.parseInt(CampiSingolaLinea[2]);

                    // Creo una nuova istanza di attore con i dati che ho appena estratto dal file
                    Attore istanzaAttore = new Attore(codiceUnivoco, nome, dataNascita);

                    // Inserisco nella Mappa <codide identificativo univoco, istanza Attore>
                    // il codice appena estratto dalla riga corrente del file e l'istanza
                    // della classe Attore appena creata
                    attori.put(codiceUnivoco, istanzaAttore);
                }

                // Chiudo il BufferedReader e rilascio le risorse associate al file
                bufRead.close();
            }
            catch(Exception eccezione){
                System.err.println("Rilevato Errore: " + eccezione.getMessage());
                System.exit(1);
            }
        }
    
      
    private static void ScriviInFileNomi(Map<Integer,Attore> attori){
        try{
            BufferedWriter buffWriter = new BufferedWriter(new FileWriter("nomi.txt"));

            // keySet() mi restituisce un Set con le chiavi della mappa
            // e creo un ArrayList (converto il Set in una List) perché le List
            // possono essere ordinate
            List<Integer> listaCodici = new ArrayList<>(attori.keySet());
            
            // Ordino in modo crescente i codici identificatori degli attori
            Collections.sort(listaCodici);

            // Stampo la lista, prendo codice per codice
            for(int cod : listaCodici){

                // Metto in "attoreAppoggio" l'attore associato al codice "cod"
                // questo lo fa la ".get(cod)"
                Attore attoreAppoggio = attori.get(cod);
                buffWriter.write(attoreAppoggio.getCodice() + "\t" + attoreAppoggio.getNome() + "\t" + attoreAppoggio.getData());
                buffWriter.newLine();
            }
            
            // Chiudo il PrintWtiter
            buffWriter.close();
        }
        catch (Exception eccezione){
            System.err.println("Rilevato Errore: " + eccezione.getMessage());
            System.exit(1);
        }
    }
    
    
    private static void AggiungiCoprotagonisti(List<Integer> cast, Map<Integer,Attore> attori){
        // Calcolo la dimensione della lista cast
        int dimensione = cast.size();

        // Per ogni coppia di attori a1,a2 del cast, aggiungo ogni attore nel rispettivo Set coprotagonisti
        for(int i = 0; i < dimensione; i++){
            for(int j = i + 1; j < dimensione; j++){

                // Prendo il codice dell'attore a1
                int codiceAttore1 = cast.get(i);

                // Prendo il codice dell'attore a2
                int codiceAttore2 = cast.get(j);

                // Controllo sul valore del codice dei due attori, se sono uguali non li inserisco e salto
                // al ciclo successivo
                if(codiceAttore1 == codiceAttore2)
                {
                    continue;
                }

                // Inserisco nel Set progatonisti dell'attore 1 il codice univoco dell'attore 2
                attori.get(codiceAttore1).addCoprotagonista(codiceAttore2);

                // Inserisco nel Set protagonisti dell'attore 2 il codice univoco dell'attore 1
                attori.get(codiceAttore2).addCoprotagonista(codiceAttore1);
            }
        }
    }
    
    
    private static void AggiungiPartecipazioniFilm(List<Integer> cast, Map<Integer,Attore> attori, Integer codiceFilm){
        // Calcolo la dimensione del cast
        int dimensione = cast.size();

        for(int i = 0; i < dimensione; i++){

            // Ottengo il codice dell'i-esimo attore del cast
            int codiceAttore = cast.get(i);

            // Prendo l'istanza di Attore corrispondente al codice i-esimo e aggiungo
            // al suo set dei film cui a presso parte il codice del film in questione 
            attori.get(codiceAttore).addPartecipazioneFilm(codiceFilm);
        }
    }
    
    
    private static void LeggiDaFileTitoli(String nomefile, Map<Integer,Attore> attori){
        try{
            BufferedReader buffRead = new BufferedReader(new FileReader(nomefile));

            // Salto la prima riga di intestazione (cioè: tconst ordering nconst category job characters)
            buffRead.readLine();
            
            // Dichiaro la mappa chiave valore, dove la chiave è l'identificativo del film
            // mentre il valore sono gli identificativi degli attori che ci hanno lavorato
            Map<Integer,List<Integer>> titoliFilmCast = new HashMap<>();

            String lineaFile;
            while((lineaFile = buffRead.readLine()) != null){

                // Creo un array di stringhe i cui elementi sono i campi del file, saltanto
                // il delimitatore "\t"
                String[] CampiLineaFile = lineaFile.split("\t");
                
                // Dichiaro "codiceUnivoco" e lo inizializzo come: assegno a codiceUnicovo la
                // stringa corrispondente al campo 0 (cioè tconst), levo le due lettere iniziali tramite
                // il metodo ".substring(2)", e la converto in intero tramite il metodo ".parseInt()"
                int codiceUnivocoFilm = Integer.parseInt(CampiLineaFile[0].substring(2));

                // Assegno a "codiceUnivocoAttore" la stringa corrispondende al campo 2 (cioè nconst)
                // senza i primi due caratteri (ovvero nm) grazie al metodo ".substring(2)" e poi
                // la converte in un numero intero grazie al metodo "parseInt"
                int codiceUnivocoAttore = Integer.parseInt(CampiLineaFile[2].substring(2));

                // Controllo se nella mappa Map<Integer,Attore> è presente il codice univoco dell'attore
                // "containsKey(<chiave>)" controlle se una specifica chiave si trova all'interno di una
                // mappa e restituisce un valore boolenano: true se la chiave è presebte, false se la chiave
                // non è presente
                if(attori.containsKey(codiceUnivocoAttore)){
                    
                    // Se l'attore è nella mappa Map<Integer,Attore> allora controllo che il
                    // codice univo del film non sia già presente nella mappa Map<Integer,List<Integer>>
                    // se non è presente aggiungo il film alla mappa.
                    // Altrimenti aggiungo direttamente tale attore al cast del film esaminato
                    if(!titoliFilmCast.containsKey(codiceUnivocoFilm)){
                        
                        // Creo la nuova coppia <chiave,valore>
                        titoliFilmCast.put(codiceUnivocoFilm, new ArrayList<>());
                    }

                    // Dalla mappa prendo la coppia <chiave,valore> che ha come chiave il
                    // codice univo del film corrente e aggiungo alla sua lista del cast il
                    // codice attore corrente 
                    titoliFilmCast.get(codiceUnivocoFilm).add(codiceUnivocoAttore);
                }
            }

            // Chiudo il BufferedReader e rilascio le risorse associate al file
            buffRead.close();
            
            // Map.Entry<...> <nome> mi rappresenta una singola coppia <chiave, valore> della mappa
            // questa la ottengo grazie al metodo ".entrySet()" che restituisce un Set con tutte le
            // coppie <chiave, valore> e tramite il "for each" sono in grando di processare singolarmente
            // ognuna di queste coppie <chiave, valore>
            for(Map.Entry<Integer, List<Integer>> titoloCast : titoliFilmCast.entrySet()){
                //List<Integer> cast = titoloCast.getValue();
                //Integer codiceFilm = titoloCast.getKey();

                AggiungiCoprotagonisti(titoloCast.getValue(), attori);
                AggiungiPartecipazioniFilm(titoloCast.getValue(), attori, titoloCast.getKey());
            }
        }
        catch(Exception eccezione){
            System.err.println("Rilevato Errore: " + eccezione.getMessage());
            System.exit(1);
        }
    }
    
    
    private static void ScriviInFileGrafo(Map<Integer,Attore> attori){
        try{
            BufferedWriter buffWriter = new BufferedWriter(new FileWriter("grafo.txt"));

            // Dichiro e inizializzo la lista di interi "listaNodi".
            // Questa prende tutte le chiavi della mappa Map<Integer,Attori>
            // e le restituisce in un Set. Questo viene convertito in una Lista
            // (tramite new ArrayList<>) questo perché così posso ordinare
            // i nodi in ordine crescente
            List<Integer> listaNodi = new ArrayList<>(attori.keySet());     //<------- riguarda che un serve ad un cazzo

            // Ordino in modo crescente i nodi
            Collections.sort(listaNodi);

            //
            for(Integer codiceA : listaNodi){

                // Metto in "attoreAppoggio" l'istanza dell'attore associato al codice "codiceA"
                Attore attoreAppoggio = attori.get(codiceA);

                // Mi faccio restituire da "fornisciCoprotagonisti" il Set di attori coprotagonisti
                // realitco ad "attoreAppoggio", lo converto in una lista, in modo tale da poter
                // ordinare i codici degli attori coprotagonisti in ordine crescente
                List<Integer> listaCoprotagonisti = new ArrayList<>(attoreAppoggio.fornisciCoprotagonisti());
                
                // Ordino in modo crescente la lista di coprotagonisti
                Collections.sort(listaCoprotagonisti);

                // Scrivo parzialmente la linea corrente di grafo.txt
                buffWriter.write(attoreAppoggio.getCodice() + "\t" + listaCoprotagonisti.size());

                // Scrivo nella linea ogni codice di ogni coprotagonista pressente in "listaCoprotagonista"
                for(Integer codiceCoprotag : listaCoprotagonisti){
                    buffWriter.write("\t" + codiceCoprotag);
                }
                buffWriter.newLine();
            }

            buffWriter.close();
        }
        catch(Exception eccezione){
            System.err.println("Rilevato Errore: " + eccezione.getMessage());
            System.exit(1);
        }
    }
    
    
    private static void ScriviInFilePartecipazioni(Map<Integer,Attore> attori){
        try{
            BufferedWriter buffWriter = new BufferedWriter(new FileWriter("partecipazioni.txt"));

            // Restituisce un Set con tutte le chiavi della mappa, in questo mddo posso organizzarle
            // come ArrayList così da poterle ordinare in modo crescente
            List<Integer> listaAttori = new ArrayList<>(attori.keySet());

            // Ordino i codici identificativi degli attori
            Collections.sort(listaAttori);

            // Per ogni codice attore eseguo il corpo del for
            for(Integer codiceAttore : listaAttori){

                // Metto in "attoreAppoggio" l'istanza dell'attore associato al codice "codiceAttore"
                Attore attoreAppoggio = attori.get(codiceAttore);

                // Mi faccio restituire da "fornisciPartecipazioneFilm" il Set dei film cui ha partecipato
                // "attoreAppoggio", lo converto in una lista, in modo tale da poter ordinare i codici
                // dei film in ordine crescente
                List<Integer> listaFilm = new ArrayList<>(attoreAppoggio.fornisciPartecipazioneFilm());

                // Ordino la lista dei codici dei film in ordine crescente
                Collections.sort(listaFilm);

                // Inizio a scrivere nel file
                buffWriter.write(attoreAppoggio.getCodice() + "\t" + listaFilm.size());

                for(Integer codiceFilm : listaFilm){
                    buffWriter.write("\t" + codiceFilm);
                }
                buffWriter.newLine();
            }
            buffWriter.close();
        }
        catch(Exception eccezione){
            System.err.println("Rilevato Errore: " + eccezione.getMessage());
            System.exit(1);
        }
    }
    

    public static void main(String[] args){
        
        if(args.length != 2){
            System.out.println("Uso: CreaGrafo name.basics.tvs title.principals.tsv\n");
            System.exit(1);
        }
        
        // Crezione Mappa Map<Integer,Attore> dove come intero avremo il codice univo di ogni attore
        // e in corrisponzenza di Attore avremo l'istanza della classe Attore cui corrisponde il
        // codice univo specificato come primo argomento della "coppia"
        Map<Integer,Attore> coppieIdAttore = new HashMap<>();
        
        LeggiDaFileNomi(args[0], coppieIdAttore);
        
        ScriviInFileNomi(coppieIdAttore);
        
        LeggiDaFileTitoli(args[1], coppieIdAttore);
        
        ScriviInFileGrafo(coppieIdAttore);
        
        ScriviInFilePartecipazioni(coppieIdAttore);        
    }
}