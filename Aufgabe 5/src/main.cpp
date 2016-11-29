#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <limits>
using namespace std;

const double INFTY = std::numeric_limits<double>::max();
const double buhnenBreite = 70;
const double speedMax = (double) 30 / 3.6 ;		// 30 km/h ^= 30/3.6 m/s
const double speedMinnie = ( double)20 / 3.6;	// 20 km/h ^= 20/3.6 m/s

typedef pair<int,double> edge; // (target, weight)

struct node
{
	double x,y;			// Koordinaten der Lücke
	set<edge> edges;	// Ausgehende Kanten
	bool isMini;		// ist Minnielücke?

	node( double _x, double _y, set<edge> _edges, bool _isMini )
		: x(_x), y(_y), edges(_edges), isMini(_isMini) {}
};

string help_string = "Usage:\nExecutable <filename>\nfilename:\t (relativer) Pfad zur Datendatei.\n\n" ;

vector<double> getShortestPathsMax ( const vector<node>& nodes, const int startNode )
{ // modifizierte Variante von Dijkstra's algorithmus für single-source shortest paths
	vector<double> dists (nodes.size(), INFTY );
	dists[startNode] = 0;

	set<pair<double, int>> priority_queue; // [(cost, node)]
	priority_queue.insert( { 0, startNode } );

	while( !priority_queue.empty() )
	{
		int current = priority_queue.begin()->second;
		priority_queue.erase( priority_queue.begin() ); // ^= poll

		if ( nodes[current].isMini )
			continue; // Ausgehende Kanten von Minnnielücken nicht beachten

		for( auto edge : nodes[current].edges )
			if ( dists[edge.first] > dists[current] + edge.second )
			{ // Durch den aktuellen Knoten wird ein Knoten [edge.first] kürzer erreicht als bisher bekannt
				priority_queue.erase( { dists[edge.first], edge.first } ); // alten Eintrag aus Queue entfernen, sofern bereits drinnen
				dists[edge.first] = dists[current] + edge.second;
				priority_queue.insert( { dists[edge.first], edge.first} );
			}
	}

	return dists;
}

double dist ( const node& a, const node& b ) { return sqrt( pow(a.x-b.x, 2) + pow(a.y-b.y,2) ); }

bool dfs(
	const vector<int> path,						// Menge aller bereits besuchten Knoten
	const int weightSum,						// bisherige Summe der Kantengewichte auf diesem Pfad
	const int lastRow,							// [X-Koordinate der letzten Buhnenreihe			]
	const vector<node>& nodes,					// [Knoten des Graphen								]
	const vector<double>& shortestPaths_max,	// [Kürzeste Wege von Max zu jedem Knoten			]
	vector<int>& solutionPath )					// [Lösungspfad, in den bei Erfolg geschrieben wird	]
{
	int current = path.back();

	if ( nodes[current].x == lastRow )
	{ // letzte Buhnenreihe erreicht; Erfolg
		solutionPath = path;
		return true;
	}

	for( auto e : nodes[current].edges )
		if ( ((weightSum + e.second) / speedMinnie) < (shortestPaths_max[e.first] / speedMax) )
		{ // Minnie kann [e.first] in kürzerer Zeit erreichen als Max
			vector<int> tempPath = path; tempPath.push_back(e.first);
			if (
				std::find(path.begin(), path.end(), e.first) == path.end() // Zur Zyklenvermeidung. [Undone, da nicht möglich?]
				&& dfs( // Rekursiver Aufruf
					tempPath,
					weightSum + e.second,
					lastRow,
					nodes,
					shortestPaths_max,
					solutionPath
				)
			) return true;
		}

	return false;
}

int main(int argc, char* argv[])
{

	if ( argc == 0 || argc == 1 )
	{
		cerr << endl << "Fehler - Kein Dateiname übergeben. 1 Parameter: Dateiname.";
		cout << endl << help_string;
		return EXIT_FAILURE;
	}

	string filename = argv[1];

	vector<node> nodes; // Knoten

	int start_minnie, start_max; // IDs der Startknoten der Hunde
	double mostRightColumn = 0; // X-Koordinate der letzten Buhnenreihe bzw. -spalte

	// *** Einlesen der Knoten/Lücken *** //
	ifstream fin( filename );

	cout << endl << "Lade Datei '" << filename << "'" << endl;

	if (!fin.good()) 
		{ cerr << "Fehler beim Einlesen der Datendatei '" << filename << "'" << endl; exit(0); }

	while( true )
	{
		char c; double x,y;
		fin >> c >> x >> y;
		if ( fin.eof() ) break;

		if ( c == 'X' ) start_max = nodes.size();
		if ( c == 'M') start_minnie = nodes.size();
		mostRightColumn = max( mostRightColumn, x);

		nodes.push_back( node( x, y, set<edge>(), (c == 'm' || c == 'M') ) );
	}

	if (start_max == 0 && start_minnie == 0)
		cerr << endl << "Warnung - Beide Startpositionen 0; fehler in Eingabedatei?";

	if ( mostRightColumn == 0 )
		cerr << endl << "Warnung - Letzte Buhnenreihe hat X-Koordinate 0 (???)";

	double start_t = clock(); // Laufzeit stoppen

	// *** Erstellen der Kanten *** //
	for( auto &i : nodes )
		for( int j = 0; j < nodes.size(); j++ )
			/* if (abs( i.x - nodes[j].x ) == buhnenBreite
				|| ( i.x == nodes[j].x && i.y != nodes[j].y )
			) */
				// i.edges.insert( make_pair(j, dist(i, nodes[j]) ) );
			if ( (i.x + buhnenBreite == nodes[j].x) ) // nächste Buhne rechts oder
				i.edges.insert( make_pair(j, dist(i, nodes[j]) ) );

	// Ausgeben der Knoten
	cout << endl << "Startpunkt Minnie:\t(" << nodes[start_minnie].x << "," << nodes[start_minnie].y << ")";
	cout << endl << "Startpunkt Max: \t(" << nodes[start_max].x << "," << nodes[start_max].y << ")";
	cout << endl << "Letzte Buhnenreihe (X):\t" << mostRightColumn;
	
	cout << endl << "Lücken: " << nodes.size();
	// outputNodes(nodes);

	// *** Kürzeste Pfade für Max ermitteln *** //
	vector<double> shortestPaths_max	= getShortestPathsMax (nodes, start_max);
	
	cout << endl << endl << "Suche Minnieweg, bitte warten...";

	// *** Tiefensuche nach Pfad für Minnie *** //
	vector<int> minnieweg; // gefundener Lösungsweg / "Minnieweg"

	dfs( vector<int>(1,start_minnie), 0, mostRightColumn, nodes, shortestPaths_max, minnieweg );

	double end_t = clock();
	cout << ".fertig; " << (double) (end_t - start_t) / CLOCKS_PER_SEC << " Sek." << endl;

	// *** Ausgabe des Minnieweges (sofern gefunden) *** //
	if ( minnieweg.size() != 0 ) {
		cout << endl << "Minnieweg gefunden: (" << minnieweg.size() << " Lücken)" << endl;
		for( int i = 0; i < minnieweg.size(); i++ )
			cout << ( i == 0 ? "" : ", " ) << "(" << nodes[minnieweg[i]].x << "," << nodes[minnieweg[i]].y << ")";
	}else{
		cout << endl << "Kein Minnieweg gefunden..";
	}

	cout << endl;
	return EXIT_SUCCESS;

}
