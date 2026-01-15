#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// Reprezentacja skierowanego grafu używanego w całym zadaniu.
// node_count    – liczba wierzchołków po normalizacji do zakresu 0..N-1
// index_offset  – informacja, czy wejściowe pliki używały indeksowania od 0 czy od 1
// edges         – lista łuków w postaci (u, v), z indeksami 0‑bazowymi
// out_neighbors – listy następców; out_neighbors[u] to wszyscy sąsiedzi osiągalni pojedynczym łukiem z u
// in_neighbors  – listy poprzedników; in_neighbors[v] to wszyscy u, z których wychodzi łuk do v
struct Graph {
    size_t node_count{};
    size_t index_offset{};
    vector<pair<size_t, size_t>> edges;
    vector<vector<size_t>> out_neighbors;
    vector<vector<size_t>> in_neighbors;
};

// Obcina białe znaki z początku i końca napisu.
// Używamy prostej pętli while zamiast wywołań algorytmów STL,
// żeby było łatwiej prześledzić kolejne kroki.
void trim_spaces(string &text) {
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t' || text.front() == '\n' ||
                             text.front() == '\r')) {
        text.erase(text.begin());
    }
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\n' ||
                             text.back() == '\r')) {
        text.pop_back();
    }
}

// Sprawdza, czy napis składa się wyłącznie z cyfr.
// Dzięki temu możemy rozróżnić pusty Enter od numeru grafu.
bool is_number_string(const string &text) {
    if (text.empty()) {
        return false;
    }
    for (size_t i = 0; i < text.size(); ++i) {
        if (!isdigit(static_cast<unsigned char>(text[i]))) {
            return false;
        }
    }
    return true;
}

// Funkcja pomocnicza: buduje pełną strukturę Graph z listą sąsiedztwa.
// Tutaj celowo używamy prostych pętli i kopiujemy dane,
// żeby kod był maksymalnie czytelny dla początkującego.
Graph make_graph(size_t node_count, size_t index_offset, vector<pair<size_t, size_t>> edges) {
    Graph graph{};
    graph.node_count = node_count;
    graph.index_offset = index_offset;
    graph.edges = edges;
    graph.out_neighbors.clear();
    graph.in_neighbors.clear();
    graph.out_neighbors.resize(node_count);
    graph.in_neighbors.resize(node_count);

    // Każdy łuk dopisujemy ręcznie do listy następników i poprzedników.
    for (size_t i = 0; i < graph.edges.size(); ++i) {
        const size_t from = graph.edges[i].first;
        const size_t to = graph.edges[i].second;
        if (from >= node_count || to >= node_count) {
            throw runtime_error("Edge endpoint exceeds node count.");
        }
        graph.out_neighbors[from].push_back(to);
        graph.in_neighbors[to].push_back(from);
    }

    return graph;
}

// Pomocnicza funkcja do ładnego wypisywania listy ścieżek, na przykład w treści wyjątku.
// Łączy wektor ścieżek w jeden napis, rozdzielając elementy przecinkami.
string join_paths(const vector<string> &paths) {
    string joined;
    for (size_t i = 0; i < paths.size(); ++i) {
        if (i > 0) {
            joined += ", ";
        }
        joined += paths[i];
    }
    return joined;
}

// Klasyczna struktura zbiorów rozłącznych (Disjoint Set Union, Union-Find).
// Używana do "sklejania" końców łuków podczas odtwarzania grafu H.
class DisjointSet {
public:
    explicit DisjointSet(size_t size) : parent(size) {
        // parent[i] = i na start, każdy element w swoim zbiorze.
        // Robimy to jawnie w pętli, żeby było jasne, co się dzieje.
        for (size_t i = 0; i < parent.size(); ++i) {
            parent[i] = i;
        }
    }

    // Znalezienie reprezentanta zbioru z kompresją ścieżki.
    size_t find(size_t value) {
        if (parent[value] == value) {
            return value;
        }
        parent[value] = find(parent[value]);
        return parent[value];
    }

    // Połączenie dwóch zbiorów; nie dbamy tutaj o rangę,
    // bo grafy są małe i proste połączenie wystarcza.
    void unite(size_t a, size_t b) {
        const size_t root_a = find(a);
        const size_t root_b = find(b);
        if (root_a == root_b) {
            return;
        }
        parent[root_b] = root_a;
    }

private:
    vector<size_t> parent;
};

// Wspólna funkcja parsująca graf z dowolnego strumienia wejściowego:
//  - odczytuje nagłówek N M,
//  - odczytuje wszystkie łuki,
//  - wykrywa, czy wierzchołki są numerowane od 0 czy od 1,
//  - normalizuje numery wierzchołków do 0..N-1,
//  - tworzy strukturę Graph.
// Pseudokod:
// 1. Wczytaj nagłówek (N, M). W razie błędu przerwij z wyjątkiem.
// 2. Wczytaj M łuków do listy edges, sprawdzając przy okazji,
//    czy pojawił się wierzchołek o numerze 0.
// 3. Ustal index_offset = 0, jeśli pojawiło się 0; w przeciwnym razie 1.
// 4. Dla każdego łuku odejmij offset od obu końców (normalizacja),
//    pilnując, by numery nie wychodziły poza zakres 0..N-1.
// 5. Wywołaj make_graph z tak przygotowanymi danymi.
Graph parse_graph_stream(istream &graph_stream, const string &source_label) {
    size_t node_count = 0;
    size_t edge_count = 0;
    graph_stream >> node_count >> edge_count;
    if (!graph_stream) {
        throw runtime_error("Invalid header in graph file: " + source_label);
    }

    // Wczytujemy wszystkie łuki do prostego wektora.
    // Nie używamy rezerwacji pamięci ani zaawansowanych konstrukcji.
    vector<pair<size_t, size_t>> edges;

    bool uses_zero_index = false;
    for (size_t i = 0; i < edge_count; ++i) {
        size_t u = 0;
        size_t v = 0;
        graph_stream >> u >> v;
        if (!graph_stream) {
            throw runtime_error("Invalid edge list in graph file: " + source_label);
        }
        edges.push_back({u, v});
        if (u == 0 || v == 0) {
            uses_zero_index = true;
        }
    }

    // Jeśli w którymś łuku pojawiło się 0, wejście jest 0‑indeksowe.
    // W przeciwnym razie traktujemy wejście jako 1‑indeksowe.
    const size_t index_offset = uses_zero_index ? 0 : 1;
    vector<pair<size_t, size_t>> normalized_edges;

    // Normalizujemy każdy łuk w prostej pętli.
    for (size_t i = 0; i < edges.size(); ++i) {
        const size_t u_raw = edges[i].first;
        const size_t v_raw = edges[i].second;

        if (u_raw < index_offset || v_raw < index_offset) {
            throw runtime_error("Edge endpoint below expected offset in " + source_label);
        }
        const size_t u = u_raw - index_offset;
        const size_t v = v_raw - index_offset;
        if (u >= node_count || v >= node_count) {
            throw runtime_error("Edge endpoint exceeds declared node count in " + source_label);
        }
        normalized_edges.push_back({u, v});
    }

    return make_graph(node_count, index_offset, normalized_edges);
}

// Funkcja, która próbuje wczytać graf z kilku możliwych ścieżek.
// Zwraca pierwszy poprawnie wczytany graf oraz zapisuje w resolved_path
// faktycznie użyty plik.
// Pseudokod:
// 1. Dla każdej ścieżki z listy candidate_paths:
//      a) spróbuj otworzyć plik,
//      b) jeżeli się uda – wczytaj graf i zapamiętaj ścieżkę w resolved_path,
//         po czym natychmiast zwróć graf.
// 2. Jeśli żadna ścieżka nie zadziała, wyrzuć wyjątek z listą prób.
Graph load_graph(const vector<string> &candidate_paths, string &resolved_path) {
    vector<string> attempted_paths;
    for (const auto &path : candidate_paths) {
        attempted_paths.push_back(path);
        ifstream graph_file(path);
        if (!graph_file.is_open()) {
            continue;
        }
        resolved_path = path;
        return parse_graph_stream(graph_file, path);
    }

    throw runtime_error("Unable to open input graph file. Tried: " + join_paths(attempted_paths));
}

// Zapisuje graf do podanego pliku w tym samym formacie, co na wejściu:
// najpierw N M, a potem wszystkie łuki. Dodajemy offset, aby wrócić
// do oryginalnej numeracji (0‑ lub 1‑indeksowej).
// Pseudokod:
// 1. Otwórz plik docelowy w trybie zapisu.
// 2. Zapisz nagłówek: node_count oraz liczbę łuków.
// 3. Dla każdego łuku (u, v):
//      a) dodaj output_offset do obu końców, żeby wrócić do oryginalnej numeracji,
//      b) zapisz parę w pliku.
void save_graph(const Graph &graph, const string &output_path, size_t output_offset) {
    ofstream graph_file(output_path);
    if (!graph_file.is_open()) {
        throw runtime_error("Unable to open output file: " + output_path);
    }

    graph_file << graph.node_count << ' ' << graph.edges.size() << '\n';
    for (const auto &edge : graph.edges) {
        graph_file << edge.first + output_offset << ' ' << edge.second + output_offset << '\n';
    }
}

// Podobnie jak load_graph, tylko dla zapisu: próbujemy kilku ścieżek
// i zwracamy tę, która zadziałała. Jeśli żadna się nie uda, wyrzucamy wyjątek.
// Pseudokod:
// 1. Dla każdej ścieżki z candidate_paths spróbuj zapisać graf:
//      a) jeśli zapis się uda – zwróć tę ścieżkę,
//      b) jeśli się nie uda – zapamiętaj ostatni błąd i spróbuj następnej.
// 2. Jeżeli żadna ścieżka się nie powiodła, wyrzuć wyjątek (z ostatnim błędem, jeśli istnieje).
string save_graph_with_candidates(const Graph &graph,
                                  const vector<string> &candidate_paths,
                                  size_t output_offset) {
    string last_error;
    for (const auto &path : candidate_paths) {
        try {
            save_graph(graph, path, output_offset);
            return path;
        } catch (const exception &ex) {
            last_error = ex.what();
        }
    }

    if (!last_error.empty()) {
        throw runtime_error(last_error);
    }
    throw runtime_error("Unable to save graph to any of the requested paths.");
}

// Główny pomysł odtwarzania grafu H:
//  * każdy wierzchołek G traktujemy jako łuk w H (to odwraca proces budowania grafu liniowego),
//  * dla wierzchołka i w G tworzymy dwa "końce" łuku w H: tail_ids[i] (początek) i head_ids[i] (koniec),
//  * każda krawędź (u, v) w G mówi, że koniec łuku u styka się z początkiem łuku v, więc łączymy je w DSU,
//  * po sklejeniu grupy końców stają się wierzchołkami H; odczytujemy numery z DSU i tworzymy łuki H.
// Na papierze to odpowiedź na pytanie: „Jaki graf H ma graf liniowy równy G?”.
Graph build_candidate_original(const Graph &conjugated) {
    if (conjugated.node_count == 0) {
        return make_graph(0, 0, {});
    }

    const size_t endpoint_count = conjugated.node_count * 2;
    DisjointSet disjoint_set(endpoint_count);

    // Dla wierzchołka i w G przypisujemy:
    //  * tail_ids[i] – "początek" łuku w H,
    //  * head_ids[i] – "koniec" łuku w H.
    vector<size_t> tail_ids(conjugated.node_count);
    vector<size_t> head_ids(conjugated.node_count);
    for (size_t vertex = 0; vertex < conjugated.node_count; ++vertex) {
        tail_ids[vertex] = vertex * 2;
        head_ids[vertex] = vertex * 2 + 1;
    }

    // Dla każdego łuku (u, v) w G sklejamy koniec łuku odpowiadającego u
    // z początkiem łuku odpowiadającego v w H.
    // Łączenie końców: koniec łuku u sklejamy z początkiem łuku v.
    for (size_t i = 0; i < conjugated.edges.size(); ++i) {
        const size_t head_id = head_ids[conjugated.edges[i].first];
        const size_t tail_id = tail_ids[conjugated.edges[i].second];
        disjoint_set.unite(head_id, tail_id);
    }

    // Przydzielamy nowe numery wierzchołkom H na podstawie reprezentantów z DSU.
    vector<size_t> root_to_id(endpoint_count, numeric_limits<size_t>::max());
    size_t next_id = 0;

    // Teraz każdy wierzchołek G (każdy "łuk") staje się jednym łukiem w H.
    vector<pair<size_t, size_t>> edges;
    for (size_t vertex = 0; vertex < conjugated.node_count; ++vertex) {
        // tail_root / head_root to identyfikatory grup w DSU.
        const size_t tail_root = disjoint_set.find(tail_ids[vertex]);
        if (root_to_id[tail_root] == numeric_limits<size_t>::max()) {
            root_to_id[tail_root] = next_id++;
        }
        const size_t tail = root_to_id[tail_root];

        const size_t head_root = disjoint_set.find(head_ids[vertex]);
        if (root_to_id[head_root] == numeric_limits<size_t>::max()) {
            root_to_id[head_root] = next_id++;
        }
        const size_t head = root_to_id[head_root];

        edges.push_back({tail, head});
    }

    // index_offset = 0, bo H przechowujemy wewnętrznie jako 0‑indeksowy.
    return make_graph(next_id, 0, edges);
}

// Buduje graf liniowy (sprzężony) z grafu H:
//  * wierzchołki to łuki H,
//  * istnieje łuk z i do j, jeśli koniec łuku i jest początkiem łuku j.
// Pseudokod:
// 1. Dla każdego łuku edge_i=(u_i, v_i) w grafie H:
//      a) dla każdego łuku edge_j=(u_j, v_j) sprawdź, czy v_i == u_j,
//      b) jeśli tak, dodaj łuk i -> j do grafu liniowego.
// 2. Zwróć graf, w którym liczba wierzchołków = liczba łuków H.
Graph build_line_digraph(const Graph &graph) {
    vector<pair<size_t, size_t>> edges;
    const size_t vertex_count = graph.edges.size();

    for (size_t i = 0; i < vertex_count; ++i) {
        const pair<size_t, size_t> &edge_i = graph.edges[i];
        const size_t head_i = edge_i.second;
        for (size_t j = 0; j < vertex_count; ++j) {
            const pair<size_t, size_t> &edge_j = graph.edges[j];
            const size_t tail_j = edge_j.first;
            if (head_i == tail_j) {
                edges.push_back({i, j});
            }
        }
    }

    return make_graph(vertex_count, 0, edges);
}

// Porównuje dwa grafy po liczbie wierzchołków oraz multizbiorze łuków.
// Kolejność łuków w wektorze nie ma znaczenia – sortujemy i porównujemy.
// Pseudokod:
// 1. Jeżeli liczba wierzchołków lub liczba łuków się różni – zwróć false.
// 2. Posortuj listy łuków obu grafów.
// 3. Zwróć true wtedy i tylko wtedy, gdy posortowane listy są identyczne.
bool graphs_equal(const Graph &lhs, const Graph &rhs) {
    if (lhs.node_count != rhs.node_count) {
        return false;
    }
    if (lhs.edges.size() != rhs.edges.size()) {
        return false;
    }

    vector<pair<size_t, size_t>> lhs_edges = lhs.edges;
    vector<pair<size_t, size_t>> rhs_edges = rhs.edges;
    sort(lhs_edges.begin(), lhs_edges.end());
    sort(rhs_edges.begin(), rhs_edges.end());
    return lhs_edges == rhs_edges;
}

// Próbuje odzyskać graf oryginalny H z grafu G i weryfikuje wynik:
//  * buduje kandydata H,
//  * buduje L(H),
//  * sprawdza, czy L(H) = G.
// Pseudokod:
// 1. Zbuduj kandydata H z grafu G (build_candidate_original).
// 2. Zbuduj graf liniowy L(H) (build_line_digraph).
// 3. Porównaj L(H) z G (graphs_equal):
//      a) jeśli różne – zwróć false,
//      b) jeśli identyczne – wpisz kandydata do original_graph i zwróć true.
bool recover_original_graph(const Graph &graph, Graph &original_graph) {
    Graph candidate = build_candidate_original(graph);
    Graph reconstructed = build_line_digraph(candidate);
    if (!graphs_equal(graph, reconstructed)) {
        return false;
    }
    original_graph = candidate;
    return true;
}

// Sprawdza, czy graf jest tzw. „one‑grafem”, czyli nie zawiera
// łuków wielokrotnych. Innymi słowy, każdy (u, v) występuje
// w liście krawędzi co najwyżej raz.
// Pseudokod:
// 1. Skopiuj listę łuków i posortuj ją.
// 2. Przejdź po posortowanej liście:
//      a) jeśli dwa kolejne łuki są takie same -> graf nie jest one‑grafem.
// 3. Jeżeli nie znaleziono duplikatu, zwróć true.
bool is_one_graph(const Graph &graph) {
    if (graph.edges.empty()) {
        return true;
    }

    vector<pair<size_t, size_t>> edges = graph.edges;
    sort(edges.begin(), edges.end());
    for (size_t i = 1; i < edges.size(); ++i) {
        if (edges[i] == edges[i - 1]) {
            return false;
        }
    }

    return true;
}

// Sprawdza liniowość grafu H. Przyjmujemy, że liniowość oznacza, że
// każdy wierzchołek ma stopień nie większy niż 2 (licząc pętle jako 2).
// Pseudokod:
// 1. Inicjalizuj tablicę stopni (degree).
// 2. Dla każdego łuku (u, v):
//      a) jeśli u == v -> pętla (stopień +2); sprawdź, czy nie przekracza 2,
//      b) w przeciwnym razie zwiększ stopnie u i v o 1, również kontrolując limit 2.
// 3. Jeśli żaden wierzchołek nie przekroczył stopnia 2, graf jest liniowy.
bool is_linear_graph(const Graph &graph) {
    if (graph.node_count == 0) {
        return true;
    }

    vector<size_t> degree(graph.node_count, 0);
    for (const auto &edge : graph.edges) {
        const size_t u = edge.first;
        const size_t v = edge.second;
        if (u == v) {
            degree[u] += 2;
            if (degree[u] > 2) {
                return false;
            }
            continue;
        }

        ++degree[u];
        if (degree[u] > 2) {
            return false;
        }
        ++degree[v];
        if (degree[v] > 2) {
            return false;
        }
    }

    return true;
}

int main(int argc, char **argv) {
    // Zbieramy możliwe ścieżki wejścia i wyjścia:
    //  * jeśli użytkownik poda numer grafu, użyjemy graphN.txt / graph_outN.txt,
    //  * jeśli poda konkretne ścieżki w argumentach, użyjemy ich,
    //  * w przeciwnym razie korzystamy z domyślnych graph.txt / graph_out.txt.
    vector<string> input_candidates;
    vector<string> output_candidates;

    string chosen_id;
    if (argc <= 1) {
        cout << "Enter graph number (e.g., 3 for graph3.txt) or press Enter for defaults: " << flush;
        string line;
        if (getline(cin, line)) {
            // Używamy naszych prostych helperów: trim_spaces + is_number_string.
            trim_spaces(line);
            if (is_number_string(line)) {
                chosen_id = line;
            }
        }
    }

    if (!chosen_id.empty()) {
        input_candidates.emplace_back("Exercise2/graph" + chosen_id + ".txt");
        input_candidates.emplace_back("graph" + chosen_id + ".txt");
        output_candidates.emplace_back("Exercise2/graph_out" + chosen_id + ".txt");
        output_candidates.emplace_back("graph_out" + chosen_id + ".txt");
    } else if (argc > 1) {
        input_candidates.emplace_back(argv[1]);
        if (argc > 2) {
            output_candidates.emplace_back(argv[2]);
        }
    }

    if (input_candidates.empty()) {
        input_candidates.emplace_back("Exercise2/graph.txt");
        input_candidates.emplace_back("graph.txt");
    }

    if (output_candidates.empty()) {
        output_candidates.emplace_back("Exercise2/graph_out.txt");
        output_candidates.emplace_back("graph_out.txt");
    }

    try {
        // Wczytanie grafu na podstawie listy kandydackich ścieżek.
        string resolved_input;
        Graph graph = load_graph(input_candidates, resolved_input);
        cout << "Loaded graph from " << resolved_input << " with " << graph.node_count << " vertices and "
             << graph.edges.size() << " edges.\n";

        // Dodatkowa informacja: czy graf jest „one‑grafem” (bez łuków wielokrotnych).
        const bool one_graph = is_one_graph(graph);
        cout << "Graph is one-graph (no multiple edges): " << boolalpha << one_graph << '\n';

        // Próba odtworzenia grafu oryginalnego H.
        Graph original_graph;
        const bool conjugated = recover_original_graph(graph, original_graph);
        cout << "Graph is conjugated: " << boolalpha << conjugated << '\n';
        if (!conjugated) {
            // Jeśli nie jest sprzężony, kończymy – nie ma sensu próbować zapisu.
            return 0;
        }

        // Sprawdzenie liniowości odzyskanego H.
        const bool linear = is_linear_graph(original_graph);
        cout << "Original graph is linear: " << boolalpha << linear << '\n';

        // Zapis grafu H do jednej z możliwych ścieżek wyjściowych.
        const string resolved_output =
            save_graph_with_candidates(original_graph, output_candidates, graph.index_offset);
        cout << "Original graph saved to " << resolved_output << '\n';
    } catch (const exception &ex) {
        cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
