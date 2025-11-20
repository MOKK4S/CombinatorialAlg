#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
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

// Funkcja pomocnicza: na podstawie liczby wierzchołków, offsetu i listy łuków
// buduje pełną strukturę Graph wraz z listami sąsiedztwa. Po drodze sprawdza,
// czy łuki nie odwołują się do wierzchołków spoza zakresu.
// Pseudokod:
// 1. Utwórz pustą strukturę Graph i przypisz node_count oraz index_offset.
// 2. Przekopiuj listę łuków i zainicjalizuj tablice sąsiedztwa (out_neighbors, in_neighbors).
// 3. Dla każdego łuku (u, v):
//      a) sprawdź, czy indeksy mieszczą się w zakresie,
//      b) dopisz v do listy następników u,
//      c) dopisz u do listy poprzedników v.
// 4. Zwróć gotowy graf.
Graph make_graph(size_t node_count, size_t index_offset, vector<pair<size_t, size_t>> edges) {
    Graph graph{};
    graph.node_count = node_count;
    graph.index_offset = index_offset;
    graph.edges = std::move(edges);
    graph.out_neighbors.assign(node_count, {});
    graph.in_neighbors.assign(node_count, {});

    for (const auto &edge : graph.edges) {
        if (edge.first >= node_count || edge.second >= node_count) {
            throw runtime_error("Edge endpoint exceeds node count.");
        }
        graph.out_neighbors[edge.first].push_back(edge.second);
        graph.in_neighbors[edge.second].push_back(edge.first);
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
        iota(parent.begin(), parent.end(), 0);
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

    vector<pair<size_t, size_t>> edges;
    edges.reserve(edge_count);

    bool uses_zero_index = false;
    for (size_t i = 0; i < edge_count; ++i) {
        size_t u = 0;
        size_t v = 0;
        graph_stream >> u >> v;
        if (!graph_stream) {
            throw runtime_error("Invalid edge list in graph file: " + source_label);
        }
        edges.emplace_back(u, v);
        if (u == 0 || v == 0) {
            uses_zero_index = true;
        }
    }

    // Jeśli w którymś łuku pojawiło się 0, wejście jest 0‑indeksowe.
    // W przeciwnym razie traktujemy wejście jako 1‑indeksowe.
    const size_t index_offset = uses_zero_index ? 0 : 1;
    vector<pair<size_t, size_t>> normalized_edges;
    normalized_edges.reserve(edges.size());

    for (const auto &edge : edges) {
        if (edge.first < index_offset || edge.second < index_offset) {
            throw runtime_error("Edge endpoint below expected offset in " + source_label);
        }
        const size_t u = edge.first - index_offset;
        const size_t v = edge.second - index_offset;
        if (u >= node_count || v >= node_count) {
            throw runtime_error("Edge endpoint exceeds declared node count in " + source_label);
        }
        normalized_edges.emplace_back(u, v);
    }

    return make_graph(node_count, index_offset, std::move(normalized_edges));
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
//  * każdy wierzchołek G traktujemy jako łuk w H,
//  * dla każdego wierzchołka w G tworzymy dwa "końce" łuku (początek i koniec),
//  * następnie, dla każdego łuku (u, v) w G, łączymy koniec łuku odpowiadającego u
//    z początkiem łuku odpowiadającego v,
//  * po scaleniu za pomocą DSU grupujemy końce łuków w wierzchołki H.
// Pseudokod:
// 1. Jeśli graf G ma 0 wierzchołków, zwróć pusty graf H.
// 2. Dla każdego wierzchołka i w G:
//      a) przypisz identyfikatory tail_ids[i] i head_ids[i] (początek/koniec łuku H),
//      b) użyj DSU do połączenia head_ids[u] i tail_ids[v] dla każdej krawędzi (u, v) w G.
// 3. Po sklejeniu końców:
//      a) przejdź przez wszystkie tail/head i przypisz im kolejne numery wierzchołków H,
//      b) dla każdego wierzchołka G dodaj łuk (tail_id, head_id) do grafu H.
// 4. Zwróć utworzony graf H (0-indeksowy).
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
    for (const auto &edge : conjugated.edges) {
        const size_t head_id = head_ids[edge.first];
        const size_t tail_id = tail_ids[edge.second];
        disjoint_set.unite(head_id, tail_id);
    }

    // Przydzielamy nowe numery wierzchołkom H na podstawie reprezentantów z DSU.
    vector<size_t> root_to_id(endpoint_count, numeric_limits<size_t>::max());
    size_t next_id = 0;
    auto map_endpoint = [&](size_t endpoint) {
        const size_t root = disjoint_set.find(endpoint);
        size_t &assigned = root_to_id[root];
        if (assigned == numeric_limits<size_t>::max()) {
            assigned = next_id++;
        }
        return assigned;
    };

    // Teraz każdy wierzchołek G (każdy "łuk") staje się jednym łukiem w H.
    vector<pair<size_t, size_t>> edges;
    edges.reserve(conjugated.node_count);
    for (size_t vertex = 0; vertex < conjugated.node_count; ++vertex) {
        const size_t tail = map_endpoint(tail_ids[vertex]);
        const size_t head = map_endpoint(head_ids[vertex]);
        edges.emplace_back(tail, head);
    }

    // index_offset = 0, bo H przechowujemy wewnętrznie jako 0‑indeksowy.
    return make_graph(next_id, 0, std::move(edges));
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
    edges.reserve(vertex_count * 2);

    for (size_t i = 0; i < vertex_count; ++i) {
        const pair<size_t, size_t> &edge_i = graph.edges[i];
        const size_t head_i = edge_i.second;
        for (size_t j = 0; j < vertex_count; ++j) {
            const pair<size_t, size_t> &edge_j = graph.edges[j];
            const size_t tail_j = edge_j.first;
            if (head_i == tail_j) {
                edges.emplace_back(i, j);
            }
        }
    }

    return make_graph(vertex_count, 0, std::move(edges));
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
    original_graph = std::move(candidate);
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
            auto trim = [](string &text) {
                const auto first = text.find_first_not_of(" \t\r\n");
                if (first == string::npos) {
                    text.clear();
                    return;
                }
                const auto last = text.find_last_not_of(" \t\r\n");
                text = text.substr(first, last - first + 1);
            };
            trim(line);
            const bool numeric =
                !line.empty() && all_of(line.begin(), line.end(), [](unsigned char ch) { return isdigit(ch); });
            if (numeric) {
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
