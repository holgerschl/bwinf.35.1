/** Aufgabe 3 **/
#include <iostream>
#include <fstream>
#include <sstream>

#include <string>
#include <vector>
#include <queue>
#include <set>
#include <map>

#include <algorithm>

#include <cctype>
#include <ctime>
#include <cstdlib>

using namespace std;

/** "Typen" **/
typedef pair< int, int > coord;
typedef vector< vector< int > > puzzle;

/** Flags **/
bool flag_logEnabled = true;
bool flag_printSolutionFields = false;

/** Makros **/
#define F_WALL  -200
#define F_EMPTY -100

#define LOG if(flag_logEnabled) cout << endl

/** Funktionen **/

//** Ein- und Ausgabe **//
void exitError( string msg = "FEHLER: Unbehandelter Fehler aufgetreten. Beende." /* (default message) */ )
{ // Prints an error message and exits with status FAILURE 
    cerr << /* endl <<*/  msg << endl;
    exit(EXIT_FAILURE);
}

void printPuzzle( const puzzle& p )
{ // Puzzle auf der Konsole ausgeben

    cout << endl;
    for( int y = p.size() - 1; y >= 0; y-- ) {
        cout << endl;
        for( int x = 0; x < p.size(); x++ ) {
            switch( p[x][y] ) {
                case F_WALL:
                    cout << "#"; break;
                case F_EMPTY:
                    cout << " "; break;
                default:
                    cout << p[x][y]; break;
            }
        }
    }
    cout << endl;
}

puzzle readFromFile ( const string& filename )
{ // Datendatei auslesen; gibt initialen Puzzle-Status zurück
    // Datei öffnen
    ifstream fin ( filename.c_str() );
    if ( !fin.good() )
        exitError( "FEHLER: Die angegebene Datei '" + filename + "' konnte nicht geöffnet werden. Beende." );
    
    int N = 0; // Spielfeldgröße

    string line = "";
    getline( fin, line );
    istringstream iss (line);
    if (!(iss >> N ))
        exitError( "FEHLER: Fehler beim Auslesen der ersten Zeile aus der Datendatei (Feldgröße). Beende." );
    
    puzzle initialState (N, vector<int>(N)); // Startzustand des Puzzles

    for( int y = N-1; y >= 0; y--) {
        string line;

        if ( !getline( fin, line ) )
            exitError("FEHLER - Konnte Zeile Nr. " + to_string(N- (y-1) ) + " aus Datendatei nicht auslesen.");
        
        if ( line.length() != N ) // Nicht genau N Zeichen in Zeile?
            exitError("FEHLER - Zeile Nr. " + to_string(N-(y-1)) + " enthält nicht " + to_string(N) + ", sondern " + to_string(line.length()) + " Zeichen!" );

        for( int x = 0; x < N; x++) {
            string s = line.substr( x, 1); //aktuelles Zeichen auslesen

            if ( s == "#" ) { // Wand
                initialState[x][y] = F_WALL;
            }else if( s == " " ) { // Leeres Feld
                initialState[x][y] = F_EMPTY;
            }else if(isdigit(s[0])) { // Stab
                initialState[x][y] = atoi( s.c_str() );
            }else // Unbekanntes Zeichen
                exitError( "FEHLER - Unbekanntes Zeichen '" + s + "' in Datendatei! Beende." ); 
        }
    }

    return initialState;
}


//** Simulation: Drehung und Gravitation **//
puzzle turnPuzzle ( const puzzle& p, const bool clockwise = true )
{ // Drehen des Puzzles (Helferfunktion) - Keine(!) Anwendung der Gravitation 
     if ( clockwise )
     { // Drehung im Uhrzeigersinn 
        puzzle ret (p.size(), vector<int>(p.size()));

        for( int x = 0; x < p.size(); x++ )
            for( int y = 0; y < p[0].size(); y++ )
                ret[y][p.size() - x - 1] = p[x][y]; // (x,y) => (y, N - x - 1)

        return ret;
     }else
     { // 1 Drehung gegen den Uhrzeigersinn = 3 Drehungen mit dem Uhrzeigersinn
        return turnPuzzle( turnPuzzle( turnPuzzle(p, true), true), true);
     }
}

bool gforcePuzzle( puzzle& p )
{ // Wendet Gravitation auf die Stäbe eines Puzzles an und gibt zurück, ob ein Stab durch die Lücke gefallen ist

    bool stickFallen = true;
    while( stickFallen ){
        stickFallen = false;

        // Stäbe nach ihrer geringsten Höhe sortieren
        map< int, int> stickHeights;

        for( int y = p.size() - 1; y >= 0; y-- )
            for( int x = 0; x < p[0].size(); x++ )
                if ( p[x][y] != F_WALL && p[x][y] != F_EMPTY ) { // Stab, da keine Wand oder Leeres Feld
                    if ( stickHeights.find( p[x][y] ) == stickHeights.end() ) {
                        stickHeights.insert( make_pair(p[x][y], y) );
                    }else{
                        if ( stickHeights.at( p[x][y] ) > y )
                            stickHeights.at( p[x][y] ) = y;
                    }
                }

        // Nach Y-Wert sortieren
        vector< pair<int,int> > heightsSorted;
        for( auto i : stickHeights)
            heightsSorted.push_back( make_pair(i.second, i.first) );

        sort( heightsSorted.begin(), heightsSorted.end() );


        for( auto s : heightsSorted )
        { // for each stick

            // maximales Fall-Y ermitteln
            int maxFallY = -1;

            for( int x = 0; x < p.size(); x++ )
            {
                if ( p[x][s.first] != s.second ) continue;
                int y = s.first - 1; for( ; y >= 0 && p[x][y] == F_EMPTY; y-- ) {} y++;
                maxFallY = max( maxFallY, y );
            }
            
            if ( s.first > maxFallY )
            { // Dieser Stab kann fallen
                for( int x = 0; x < p.size(); x++ )
                    for( int y = 0; y < p.size(); y++ )
                        if ( p[x][y] == s.second ) {
                            p[x][y] = F_EMPTY;
                            p[x][y - abs(s.first-maxFallY) ] = s.second;
                        }

                stickFallen = true;
                break;
            }

            if ( maxFallY == 0 ) return true;

        }

    }

    return false;
}

puzzle rotatePuzzle ( const puzzle& p, const bool clockwise = true )
{ // Drehung des Puzzles in eine Richtung; Anwendung von Gravitation; gibt ein neues, gedrehtes Puzzle zurück, ohne das übergebene Puzzle zu verändern
    puzzle temp = turnPuzzle( p, clockwise );
    gforcePuzzle(temp);
    return temp;
}

bool stickReleased( const puzzle& p )
{ // Gibt true zurück, wenn ein Stab durch die Lücke / aus dem Rahmen fällt
    puzzle temp = p;
    return gforcePuzzle(temp);
}

//** Breitensuche **//
vector<bool> bfs ( const puzzle& initialState )
{ // Breitensuche
    queue< pair< puzzle, vector<bool> > > q; // Warteschlange; [(Puzzle, decisionPath)]
    set< puzzle > reachedStates;

    reachedStates.insert( initialState );
    q.push( { initialState, vector<bool>() } ); // Warteschlange mit Startzustand initialisieren; leerer Entscheidungspfad

    while( !q.empty() ) {
        pair< puzzle, vector< bool > > p = q.front(); q.pop(); // ersten Zustand aus der Liste herausnehmen
        
        // Ziel erreicht?
        if ( stickReleased(p.first) )
            return p.second; // gib den Entscheidungspfad zu diesem Zustand zurück
        
        // Zustandsveränderung
        puzzle a = rotatePuzzle( p.first, true ); // Drehung im Uhrzeigersinn
        puzzle b = rotatePuzzle( p.first, false );
        
        if ( reachedStates.find( a ) == reachedStates.end() ) {
            reachedStates.insert( a ); // mark node as reached
            vector<bool> tempPath = p.second; tempPath.push_back( true );
            q.push( make_pair( a, tempPath ));
        }
        
        if ( reachedStates.find( b ) == reachedStates.end() ) {
            reachedStates.insert( b ); // mark node as reached
            vector<bool> tempPath = p.second; tempPath.push_back( false );
            q.push( make_pair( b, tempPath ));
        }

    }
    return *( new vector<bool>() ); // Leeren Lösungspfad zurückgeben, wenn keine Lösungsmöglichkeit gefunden

}

int main(int argc, char* argv[])
{
    // Kommandozeilenparameter auswerten
    std::vector<std::string> args;
	for (int i = 1 /*(skip first arg)*/; i < argc; ++i)
		args.push_back(argv[i]);

    string filename;
    for( int i = 0; i < args.size(); i++ )
    {
        if ( args[i] == "-?" || args[i] == "--help" ) {
            // Hilfe-Ausgabe
            cout << endl << "35. Bundeswettbewerb Informatik 2016/'17, 1.Runde, Aufgabe 3 - 'Rotation'.";
            cout << endl << "Lösung von Team 'Wouldn't IT be nice...'";
            cout << endl << endl << "args: ";
            cout << endl << "-f --file\t\tData file to be opened.";
            cout << endl << "-p --printFields\tPrint the solutions step by step.";
            cout << endl;
            return EXIT_SUCCESS;
        }
        else if ( args[i] == "-f" || args[i] == "--file" ) {
            if ( i+1 >= args.size() )
                exitError( "Fehler - Kein Dateiname nach '" + args[i] + "'");
            filename = args[i+1];
            i++;
        }
        else if( args[i] == "-p" || args[i] == "--printFields" ){
            flag_printSolutionFields ^= 1; // flip bool value
        }
        else {
            cerr << endl << "Warnung: Unerkannter Kommandozeilenparameter '" << args[i] << "'. Ignoriere.";
        }
    }

    // Einlesen der Datei
    if ( filename == "" )
        exitError( "Fehler: Kein Dateiname übergeben. Verwenden Sie den Kommandozeilenparameter '-f <Dateiname>' oder '--help'. Beende." );

    cout << endl << "Lade Datei '" << filename << "'... ";

    puzzle initialState = readFromFile( filename );
    printPuzzle( initialState );
    
    LOG << "Startzustand erfolgreich geladen.";
    LOG << "Spielfeldgröße: " << initialState.size() << " x " << initialState[0].size();

    cout << endl << endl << "Suche Lösung, bitte warten... " << endl;

    double start_t = clock(); // Laufzeit stoppen
    vector<bool> solutionPath = bfs( initialState ); // Breitensuche durchführen
    double end_t = clock();

    cout << "Fertig. [" << ( (double) (end_t - start_t) / CLOCKS_PER_SEC ) << " Sek.]" << endl;

    if ( solutionPath.size() != 0)
    { // Lösungsweg gefunden
        cout << endl << "Lösungsweg gefunden (" << solutionPath.size() << " Drehung" << (solutionPath.size() > 1 ? "en" : "") << "):";

        for( int i = 0; i < solutionPath.size(); i++ )
            cout << ( ( i == 0 ) ? "" : "," ) << ( solutionPath[i] ? "rechts" : "links" );

        if ( flag_printSolutionFields )
        { // Ausgabe der Felder mit Verdrehung schrittweise

            puzzle temp = initialState;

            cout << endl << endl << endl << "Startzustand: ";
            printPuzzle( temp );

            for( auto i : solutionPath ) {
                temp = rotatePuzzle( temp, i);
                cout << endl << "Puzzle nach " << ( i ? "rechts" : "links" ) << "-Rotation:";
                printPuzzle( temp );
            }

        }

    }else if( stickReleased(initialState) )
    { // Im Startzustand fällt ein Stab aus dem Rahmen
        cout << endl << "HINWEIS - Bereits im Startzustand fällt ein Stab durch die Lücke! Keine Rotation erforderlich.";
    }else
    { // Keine Lösung gefunden
        cout << endl << "Kein Lösungsweg gefunden.";
    }

    cout << endl;
    return EXIT_SUCCESS;
}