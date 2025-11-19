#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// Struktura przechowująca skierowany graf oraz informację o offsetcie numeracji.
struct Graph {
    size_t node_count{};
    size_t index_offset{};
    vector<pair<size_t, size_t>> edges;

    size_t edge_count() const {
        return edges.size();
    }
};

// Wczytuje graf z pliku (nagłówek: liczba wierzchołków i krawędzi, następnie pary wierzchołków).
Graph load_graph_from_file(const string &path) {
    ifstream input(path);
    if (!input.is_open()) {
        throw runtime_error("Nie można otworzyć pliku wejściowego: " + path);
    }

    Graph graph{};
    size_t declared_edges{};
    if (!(input >> graph.node_count >> declared_edges)) {
        throw runtime_error("Nieprawidłowy nagłówek pliku wejściowego: " + path);
    }

    graph.edges.reserve(declared_edges);
    bool uses_zero_index = false;

    for (size_t i = 0; i < declared_edges; ++i) {
        size_t u;
        size_t v;
        if (!(input >> u >> v)) {
            throw runtime_error("Nieprawidłowa liczba krawędzi w pliku wejściowym: " + path);
        }

        graph.edges.emplace_back(u, v);
        if (u == 0 || v == 0) {
            uses_zero_index = true;
        }
    }

    graph.index_offset = uses_zero_index ? 0 : 1;
    const size_t min_index = graph.index_offset;
    const size_t max_index = graph.index_offset + graph.node_count - 1;

    for (const auto &edge : graph.edges) {
        if (edge.first < min_index || edge.first > max_index || edge.second < min_index ||
            edge.second > max_index) {
            throw runtime_error("W pliku wejściowym występują wierzchołki spoza zakresu");
        }
    }

    return graph;
}

// Zapisuje graf w tym samym formacie do wskazanego pliku.
void save_graph_to_file(const Graph &graph, const string &path) {
    ofstream output(path);
    if (!output.is_open()) {
        throw runtime_error("Nie można otworzyć pliku wyjściowego: " + path);
    }

    output << graph.node_count << ' ' << graph.edge_count() << '\n';
    for (const auto &edge : graph.edges) {
        output << edge.first << ' ' << edge.second << '\n';
    }
}

// Najprostsza implementacja struktury DSU potrzebnej do sklejanek wierzchołków.
struct DisjointSet {
    vector<size_t> parent;
    vector<size_t> rank;

    explicit DisjointSet(size_t size) : parent(size), rank(size, 0) {
        for (size_t i = 0; i < size; ++i) {
            parent[i] = i;
        }
    }

    size_t find(size_t value) {
        if (parent[value] != value) {
            parent[value] = find(parent[value]);
        }
        return parent[value];
    }

    void unite(size_t lhs, size_t rhs) {
        lhs = find(lhs);
        rhs = find(rhs);
        if (lhs == rhs) {
            return;
        }

        if (rank[lhs] < rank[rhs]) {
            swap(lhs, rhs);
        }

        parent[rhs] = lhs;
        if (rank[lhs] == rank[rhs]) {
            ++rank[lhs];
        }
    }
};

// Buduje kandydat na graf oryginalny H z grafu sprzężonego G.
Graph restore_original_graph(const Graph &graph) {
    Graph original{};
    if (graph.node_count == 0) {
        original.index_offset = 1;
        return original;
    }

    const size_t vertex_count = graph.node_count;
    const size_t endpoint_count = vertex_count * 2;
    DisjointSet disjoint_set(endpoint_count);

    const auto start_index = [](size_t vertex) {
        return vertex * 2;
    };

    const auto end_index = [](size_t vertex) {
        return vertex * 2 + 1;
    };

    for (const auto &edge : graph.edges) {
        const size_t source = edge.first - graph.index_offset;
        const size_t target = edge.second - graph.index_offset;
        disjoint_set.unite(end_index(source), start_index(target));
    }

    vector<pair<size_t, size_t>> reconstructed_edges;
    reconstructed_edges.reserve(vertex_count);
    vector<size_t> representative_to_id(endpoint_count, 0);
    size_t next_identifier = 1;

    const auto assign_identifier = [&](size_t index) {
        const size_t representative = disjoint_set.find(index);
        size_t &identifier = representative_to_id[representative];
        if (identifier == 0) {
            identifier = next_identifier++;
        }
        return identifier;
    };

    for (size_t vertex = 0; vertex < vertex_count; ++vertex) {
        const size_t start_id = assign_identifier(start_index(vertex));
        const size_t end_id = assign_identifier(end_index(vertex));
        reconstructed_edges.emplace_back(start_id, end_id);
    }

    original.node_count = next_identifier - 1;
    original.index_offset = 1;
    original.edges = std::move(reconstructed_edges);
    return original;
}

// Tworzy graf liniowy (sprzężony) z grafu oryginalnego H.
Graph build_line_graph(const Graph &original, size_t vertex_count, size_t index_offset) {
    Graph line{};
    line.node_count = vertex_count;
    line.index_offset = index_offset;

    for (size_t source_index = 0; source_index < vertex_count; ++source_index) {
        for (size_t target_index = 0; target_index < vertex_count; ++target_index) {
            if (original.edges[source_index].second == original.edges[target_index].first) {
                line.edges.emplace_back(index_offset + source_index, index_offset + target_index);
            }
        }
    }

    return line;
}

// Przygotowuje uporządkowaną listę krawędzi do porównań.
vector<pair<size_t, size_t>> sorted_edge_list(const Graph &graph) {
    vector<pair<size_t, size_t>> edges = graph.edges;
    sort(edges.begin(), edges.end());
    return edges;
}

// Sprawdza równoważność grafów (permutacje krawędzi są dozwolone).
bool are_graphs_equivalent(const Graph &lhs, const Graph &rhs) {
    if (lhs.node_count != rhs.node_count || lhs.index_offset != rhs.index_offset ||
        lhs.edge_count() != rhs.edge_count()) {
        return false;
    }

    return sorted_edge_list(lhs) == sorted_edge_list(rhs);
}

// Weryfikuje, czy graf wejściowy jest grafem sprzężonym oraz zwraca H.
bool is_conjugated_graph(const Graph &graph, Graph &original) {
    if (graph.node_count == 0) {
        original = Graph{};
        original.index_offset = 1;
        return true;
    }

    Graph candidate = restore_original_graph(graph);
    if (candidate.edges.size() != graph.node_count) {
        return false;
    }

    const Graph regenerated = build_line_graph(candidate, graph.node_count, graph.index_offset);
    if (!are_graphs_equivalent(graph, regenerated)) {
        return false;
    }

    original = std::move(candidate);
    return true;
}

// Sprawdza, czy graf jest liniowy (czyli tworzy jedną ścieżkę prostą).
bool is_linear_graph(const Graph &graph) {
    if (graph.node_count == 0) {
        return true;
    }

    vector<size_t> degree(graph.node_count, 0);
    vector<vector<size_t>> adjacency(graph.node_count);
    vector<vector<bool>> duplicate_check(graph.node_count, vector<bool>(graph.node_count, false));

    for (const auto &edge : graph.edges) {
        const size_t u = edge.first - graph.index_offset;
        const size_t v = edge.second - graph.index_offset;

        if (u >= graph.node_count || v >= graph.node_count) {
            return false;
        }

        if (u == v) {
            return false;
        }

        const size_t min_vertex = min(u, v);
        const size_t max_vertex = max(u, v);
        if (duplicate_check[min_vertex][max_vertex]) {
            return false;
        }
        duplicate_check[min_vertex][max_vertex] = true;

        ++degree[u];
        ++degree[v];
        adjacency[u].push_back(v);
        adjacency[v].push_back(u);
    }

    if (graph.node_count == 1) {
        return graph.edges.empty();
    }

    size_t degree_one_vertices = 0;
    for (const auto value : degree) {
        if (value == 1) {
            ++degree_one_vertices;
        } else if (value == 0 || value > 2) {
            return false;
        }
    }

    if (degree_one_vertices != 2) {
        return false;
    }

    queue<size_t> to_visit;
    vector<bool> visited(graph.node_count, false);
    size_t start_vertex = 0;
    while (start_vertex < graph.node_count && degree[start_vertex] == 0) {
        ++start_vertex;
    }

    if (start_vertex == graph.node_count) {
        return false;
    }

    to_visit.push(start_vertex);
    visited[start_vertex] = true;

    size_t visited_vertices = 0;
    while (!to_visit.empty()) {
        const size_t current = to_visit.front();
        to_visit.pop();
        ++visited_vertices;

        for (const size_t neighbor : adjacency[current]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                to_visit.push(neighbor);
            }
        }
    }

    return visited_vertices == graph.node_count;
}

int main(int argc, char **argv) {
    // Obsługa ścieżek wejścia/wyjścia przekazanych w argumentach.
    string input_path = "Exercise2/graph.txt";
    string output_path = "Exercise2/graph_out.txt";
    if (argc > 1) {
        input_path = argv[1];
    }
    if (argc > 2) {
        output_path = argv[2];
    }

    try {
        // Wczytanie grafu i przygotowanie struktur pomocniczych.
        const Graph graph = load_graph_from_file(input_path);
        Graph original;
        const bool conjugated = is_conjugated_graph(graph, original);
        const bool linear = conjugated && is_linear_graph(graph);

        // Raportowanie wyników testów i zapis H, jeśli to możliwe.
        cout << "Vertices: " << graph.node_count << ", edges: " << graph.edge_count() << '\n';
        cout << "Graf sprzężony: " << boolalpha << conjugated << '\n';
        if (conjugated) {
            cout << "Graf liniowy: " << boolalpha << linear << '\n';
            save_graph_to_file(original, output_path);
            cout << "Zapisano graf H do pliku: " << output_path << '\n';
        } else {
            cout << "Graf nie jest sprzężony, zapis pominięty." << '\n';
        }
    } catch (const exception &error) {
        cerr << "Błąd: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
