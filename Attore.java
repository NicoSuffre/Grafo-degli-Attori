import java.util.HashSet;
import java.util.Set;


public class Attore {
    int codice;
    String nome;
    int anno;
    private Set<Integer> coprotagonisti;
    private Set<Integer> partecipazioneFilm;
    
    // costruttore della classe Attore
    public Attore(int codice, String nome, int anno){
        this.codice = codice;
        this.nome = nome;
        this.anno = anno;

        this.coprotagonisti = new HashSet<>();
        this.partecipazioneFilm = new HashSet<>();
    }

    // Restituisce una copia del nome dell'attore
    public String getNome(){
        String copiaNome = this.nome;
        return copiaNome;
    }

    // Restituisce una copia del codice univoco dell'attore
    public int getCodice(){
        int copiaCodice = this.codice;
        return copiaCodice;
    }

    // Restituisce una copia della data di nascita dell'attore
    public int getData(){
        int copiaAnnoNascita = this.anno;
        return copiaAnnoNascita;
    }

    // Metodo per l'inserimento di un nuovo coprotagonista
    public void addCoprotagonista(Integer id){
        /* CONTROLLI SUL VALORE id */
        this.coprotagonisti.add(id);
    }

    // Metodo per restituire i coprotagonisti
    public Set<Integer> fornisciCoprotagonisti(){        
        // Restituisce una copia dell'insieme Coprotagonisti di this
        return new HashSet<>(this.coprotagonisti);
    }

    public void addPartecipazioneFilm(Integer idFilm){

        // Aggiunge all'insieme dei film cui l'attore ha preso parte,
        // il codice univoco del film in questione
        this.partecipazioneFilm.add(idFilm);
    }

    public Set<Integer> fornisciPartecipazioneFilm(){

        // Restituisce un Set che contiene tutti i codici univoci
        // identificativi di ogni film cui l'attore ha preso parte
        return new HashSet<>(this.partecipazioneFilm);
    }
}