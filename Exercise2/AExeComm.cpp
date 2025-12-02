#include <algorithm> // sort for comparing edge lists
#include <fstream>   // file input/output
#include <iostream>  // console input/output
#include <string>    // std::string for file names and input choice
#include <utility>   // std::pair for edges
#include <vector>    // std::vector for storing edges and helpers

using namespace std;

// Prosta struktura grafu: liczba wierzchołków, offset numeracji oraz lista łuków.
struct Graph {
    size_t node_count{};
    size_t index_offset{};
    vector<pair<size_t, size_t>> edges;
};

// Znajdowanie korzenia w tablicy parent bez kompresji ścieżki.
size_t find_root(vector<size_t> &parent, size_t value) {
    while (parent[value] != value) {
        value = parent[value];
    }
    return value;
}

// Łączenie dwóch zbiorów w find-union bez rang.
void unite(vector<size_t> &parent, size_t a, size_t b) {
    const size_t root_a = find_root(parent, a);
    const size_t root_b = find_root(parent, b);
    if (root_a != root_b) {
        parent[root_b] = root_a;
    }
}

// Wczytanie grafu z pliku tekstowego w formacie:
// n m
// u v
// ...
bool load_graph(const string &path, Graph &graph) {
    ifstream file(path);            // otwarcie pliku
    if (!file.is_open()) {          // jeśli nie da się otworzyć, zwróć błąd
        return false;
    }

    size_t n = 0;                   // liczba wierzchołków
    size_t m = 0;                   // liczba łuków
    file >> n >> m;                 // wczytanie nagłówka

    vector<pair<size_t, size_t>> raw_edges; // surowe łuki z pliku
    raw_edges.reserve(m);                   // rezerwacja miejsca
    bool uses_zero_index = false;           // czy numeracja startuje od 0

    for (size_t i = 0; i < m; ++i) {        // wczytanie każdego łuku
        size_t u = 0;
        size_t v = 0;
        file >> u >> v;
        raw_edges.emplace_back(u, v);
        if (u == 0 || v == 0) {             // sprawdzamy, czy użyto 0
            uses_zero_index = true;
        }
    }

    const size_t offset = uses_zero_index ? 0 : 1; // offset: 0 lub 1

    vector<pair<size_t, size_t>> edges;            // łuki znormalizowane do 0..n-1
    edges.reserve(raw_edges.size());
    for (const auto &edge : raw_edges) {           // przesunięcie indeksów o offset
        const size_t u = edge.first - offset;
        const size_t v = edge.second - offset;
        edges.emplace_back(u, v);
    }

    graph.node_count = n;       // zapis liczby wierzchołków
    graph.index_offset = offset; // zapis offsetu
    graph.edges = edges;         // zapis listy łuków
    return true;                 // sukces
}

// Wypisanie grafu na stdout w tym samym formacie co wejście.
void print_graph(const Graph &graph) {
    cout << graph.node_count << ' ' << graph.edges.size() << '\n';
    for (const auto &edge : graph.edges) { // dodajemy z powrotem offset
        cout << edge.first + graph.index_offset << ' '
             << edge.second + graph.index_offset << '\n';
    }
} 

// Sprawdzenie, czy każdy łuk ma końce w zakresie 0..node_count-1.
bool edges_in_range(const Graph &graph) {
    for (const auto &edge : graph.edges) {
        if (edge.first >= graph.node_count || edge.second >= graph.node_count) {
            return false;
        }
    }
    return true;
}

// Budowanie kandydata na graf oryginalny (H) z grafu sprzężonego (G).
Graph build_original_candidate(const Graph &conjugated) {
    Graph original{};                         // wynik startowo pusty
    if (conjugated.node_count == 0) {         // pusty graf -> pusty wynik
        return original;
    }

    const size_t endpoint_count = conjugated.node_count * 2; // po dwa końce na łuk
    vector<size_t> parent(endpoint_count);                   // tablica find-union
    for (size_t i = 0; i < parent.size(); ++i) {             // inicjalizacja
        parent[i] = i;
    }

    // Łuk (a,b) w G łączy head(a) z tail(b) w H.
    for (const auto &edge : conjugated.edges) {
        const size_t head_id = edge.first * 2 + 1;  // head wierzchołka a
        const size_t tail_id = edge.second * 2;     // tail wierzchołka b
        if (head_id < endpoint_count && tail_id < endpoint_count) {
            unite(parent, head_id, tail_id);        // sklejanie punktów
        }
    }

    vector<size_t> root_to_id(endpoint_count, static_cast<size_t>(-1)); // mapowanie korzeni
    size_t next_id = 0;                                                  // bieżący nowy indeks
    vector<pair<size_t, size_t>> edges;                                  // łuki grafu H
    edges.reserve(conjugated.node_count);

    for (size_t i = 0; i < conjugated.node_count; ++i) { // każdy wierzchołek G daje łuk w H
        const size_t tail_root = find_root(parent, i * 2);       // korzeń ogona
        const size_t head_root = find_root(parent, i * 2 + 1);   // korzeń głowy

        if (root_to_id[tail_root] == static_cast<size_t>(-1)) {  // nadanie numeru ogonowi
            root_to_id[tail_root] = next_id++;
        }
        if (root_to_id[head_root] == static_cast<size_t>(-1)) {  // nadanie numeru głowie
            root_to_id[head_root] = next_id++;
        }

        const size_t tail = root_to_id[tail_root];               // numer ogona w H
        const size_t head = root_to_id[head_root];               // numer głowy w H
        edges.emplace_back(tail, head);                          // dodanie łuku do H
    }

    original.node_count = next_id;  // liczba wierzchołków w H
    original.index_offset = 0;      // H zapisujemy od zera
    original.edges = edges;         // lista łuków H
    return original;                // zwrot kandydata H
}

// Budowa grafu liniowego L(H) z grafu H.
Graph build_line_graph(const Graph &graph) {
    Graph line_graph{};                 // wynik
    const size_t edge_count = graph.edges.size(); // liczba łuków w H

    vector<pair<size_t, size_t>> edges; // łuki w L(H)
    edges.reserve(edge_count * 2);      // prosta rezerwa

    for (size_t i = 0; i < edge_count; ++i) { // dla każdego łuku i w H
        for (size_t j = 0; j < edge_count; ++j) { // sprawdzamy każdy łuk j
            if (graph.edges[i].second == graph.edges[j].first) { // head(i)==tail(j)?
                edges.emplace_back(i, j); // wtedy (i,j) jest łukiem w L(H)
            }
        }
    }

    line_graph.node_count = edge_count; // wierzchołki L(H) to łuki H
    line_graph.index_offset = 0;        // indeksujemy od 0
    line_graph.edges = edges;           // zapisujemy łuki
    return line_graph;                  // zwrot L(H)
}

// Proste porównanie grafów: liczba wierzchołków i multizbiór łuków.
bool graphs_equal(const Graph &lhs, const Graph &rhs) {
    if (lhs.node_count != rhs.node_count) {   // porównanie liczby wierzchołków
        return false;
    }
    if (lhs.edges.size() != rhs.edges.size()) { // porównanie liczby łuków
        return false;
    }

    vector<pair<size_t, size_t>> lhs_edges = lhs.edges; // kopia łuków lhs
    vector<pair<size_t, size_t>> rhs_edges = rhs.edges; // kopia łuków rhs
    sort(lhs_edges.begin(), lhs_edges.end());           // sortowanie
    sort(rhs_edges.begin(), rhs_edges.end());           // sortowanie
    return lhs_edges == rhs_edges;                      // porównanie list
}

// Sprawdzenie, czy graf jest sprzężony i zwrócenie jego oryginału.
bool is_conjugated_graph(const Graph &graph, Graph &original_graph) {
    if (!edges_in_range(graph)) {             // węzły poza zakresem -> nie
        return false;
    }

    Graph candidate = build_original_candidate(graph); // budujemy kandydata H
    Graph rebuilt = build_line_graph(candidate);       // tworzymy L(H)
    if (!graphs_equal(graph, rebuilt)) {               // jeśli L(H)!=G -> nie jest sprzężony
        return false;
    }

    original_graph = candidate;             // zapisujemy znaleziony H
    return true;                            // sukces
}

// Sprawdzenie, czy graf jest liniowy w sensie: istnieje graf bazowy R,
// którego line graph (L(R)) jest równy grafowi wejściowemu.
bool is_line_graph(const Graph &graph) {
    Graph base_graph;                      // tu trafia ewentualny graf bazowy
    return is_conjugated_graph(graph, base_graph); // używamy tego samego testu co dla sprzężenia
}

// Zapis grafu do pliku w formacie wejściowym, z przywróceniem offsetu.
bool save_graph(const Graph &graph, const string &path, size_t output_offset) {
    ofstream file(path);                   // otwarcie pliku do zapisu
    if (!file.is_open()) {                // brak dostępu -> błąd
        return false;
    }

    file << graph.node_count << ' ' << graph.edges.size() << '\n'; // nagłówek
    for (const auto &edge : graph.edges) {                         // każdy łuk
        file << edge.first + output_offset << ' '                 // przywracamy offset
             << edge.second + output_offset << '\n';
    }
    return true;                          // sukces zapisu
}

// Główna funkcja programu.
int main() {
    cout << "Podaj numer grafu (np. 3 dla graph3.txt) lub wcisnij Enter dla domyslnego: "
         << flush;                                 // prośba o numer pliku
    string choice;
    getline(cin, choice);                          // odczyt wyboru (może być pusty)

    string input_name = "graph.txt";               // domyślny plik wejściowy
    string output_name = "graph_out.txt";          // domyślny plik wyjściowy
    if (!choice.empty()) {                         // jeśli podano numer
        input_name = "graph" + choice + ".txt";    // budujemy nazwę wejściową
        output_name = "graph_out" + choice + ".txt"; // budujemy nazwę wyjściową
    }

    Graph graph;                                   // wczytany graf G
    if (!load_graph(input_name, graph)) {          // próba wczytania
        cout << "Nie udalo sie wczytac pliku z grafem.\n";
        return 0;                                  // koniec przy błędzie
    }

    cout << "Wczytano graf z pliku " << input_name << '\n'; // informacja
    print_graph(graph);                                    // wypisanie G

    Graph original_graph;                             // graf oryginalny H
    const bool is_conjugated = is_conjugated_graph(graph, original_graph); // test sprzężenia
    cout << "Graf jest sprzezony: " << boolalpha << is_conjugated << '\n'; // wynik
    if (!is_conjugated) {                             // jeśli nie sprzężony
        return 0;                                     // kończymy
    }

    const bool is_line = is_line_graph(graph);        // test liniowości wejściowego grafu
    cout << "Graf jest liniowy: " << boolalpha << is_line << '\n'; // wynik

    if (save_graph(original_graph, output_name, graph.index_offset)) { // zapis H
        cout << "Graf oryginalny zapisany do pliku " << output_name << '\n';
    } else {                                           // błąd zapisu
        cout << "Nie udalo sie zapisac grafu do pliku.\n";
    }

    return 0;                                          // koniec programu
}
