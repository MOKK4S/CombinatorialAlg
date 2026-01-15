#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct Graph {
    size_t node_count{};
    size_t index_offset{};
    vector<pair<size_t, size_t>> edges;
};

size_t find_root(vector<size_t> &parent, size_t value) {
    while (parent[value] != value) {
        value = parent[value];
    }
    return value;
}

void unite(vector<size_t> &parent, size_t a, size_t b) {
    const size_t root_a = find_root(parent, a);
    const size_t root_b = find_root(parent, b);
    if (root_a != root_b) {
        parent[root_b] = root_a;
    }
}

bool load_graph(const string &path, Graph &graph) {
    ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    size_t n = 0;
    size_t m = 0;
    file >> n >> m;

    vector<pair<size_t, size_t>> raw_edges;
    raw_edges.reserve(m);
    bool uses_zero_index = false;

    for (size_t i = 0; i < m; ++i) {
        size_t u = 0;
        size_t v = 0;
        file >> u >> v;
        raw_edges.emplace_back(u, v);
        if (u == 0 || v == 0) {
            uses_zero_index = true;
        }
    }

    const size_t offset = uses_zero_index ? 0 : 1;

    vector<pair<size_t, size_t>> edges;
    edges.reserve(raw_edges.size());
    for (const auto &edge : raw_edges) {
        const size_t u = edge.first - offset;
        const size_t v = edge.second - offset;
        edges.emplace_back(u, v);
    }

    graph.node_count = n;
    graph.index_offset = offset;
    graph.edges = edges;
    return true;
}

void print_graph(const Graph &graph) {
    cout << graph.node_count << ' ' << graph.edges.size() << '\n';
    for (const auto &edge : graph.edges) {
        cout << edge.first + graph.index_offset << ' ' << edge.second + graph.index_offset << '\n';
    }
}

bool edges_in_range(const Graph &graph) {
    for (const auto &edge : graph.edges) {
        if (edge.first >= graph.node_count || edge.second >= graph.node_count) {
            return false;
        }
    }
    return true;
}

Graph build_original_candidate(const Graph &conjugated) {
    Graph original{};
    if (conjugated.node_count == 0) {
        return original;
    }

    const size_t endpoint_count = conjugated.node_count * 2;
    vector<size_t> parent(endpoint_count);
    for (size_t i = 0; i < parent.size(); ++i) {
        parent[i] = i;
    }

    for (const auto &edge : conjugated.edges) {
        const size_t head_id = edge.first * 2 + 1;
        const size_t tail_id = edge.second * 2;
        if (head_id < endpoint_count && tail_id < endpoint_count) {
            unite(parent, head_id, tail_id);
        }
    }

    vector<size_t> root_to_id(endpoint_count, static_cast<size_t>(-1));
    size_t next_id = 0;
    vector<pair<size_t, size_t>> edges;
    edges.reserve(conjugated.node_count);

    for (size_t i = 0; i < conjugated.node_count; ++i) {
        const size_t tail_root = find_root(parent, i * 2);
        const size_t head_root = find_root(parent, i * 2 + 1);

        if (root_to_id[tail_root] == static_cast<size_t>(-1)) {
            root_to_id[tail_root] = next_id++;
        }
        if (root_to_id[head_root] == static_cast<size_t>(-1)) {
            root_to_id[head_root] = next_id++;
        }

        const size_t tail = root_to_id[tail_root];
        const size_t head = root_to_id[head_root];
        edges.emplace_back(tail, head);
    }

    original.node_count = next_id;
    original.index_offset = 0;
    original.edges = edges;
    return original;
}

Graph build_line_graph(const Graph &graph) {
    Graph line_graph{};
    const size_t edge_count = graph.edges.size();

    vector<pair<size_t, size_t>> edges;
    edges.reserve(edge_count * 2);

    for (size_t i = 0; i < edge_count; ++i) {
        for (size_t j = 0; j < edge_count; ++j) {
            if (graph.edges[i].second == graph.edges[j].first) {
                edges.emplace_back(i, j);
            }
        }
    }

    line_graph.node_count = edge_count;
    line_graph.index_offset = 0;
    line_graph.edges = edges;
    return line_graph;
}

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

bool is_conjugated_graph(const Graph &graph, Graph &original_graph) {
    if (!edges_in_range(graph)) {
        return false;
    }

    Graph candidate =   (graph);
    Graph rebuilt = build_line_graph(candidate);
    if (!graphs_equal(graph, rebuilt)) {
        return false;
    }

    original_graph = candidate;
    return true;
}

bool is_line_graph(const Graph &graph) {
    Graph base_graph;
    return is_conjugated_graph(graph, base_graph);
}

bool save_graph(const Graph &graph, const string &path, size_t output_offset) {
    ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    file << graph.node_count << ' ' << graph.edges.size() << '\n';
    for (const auto &edge : graph.edges) {
        file << edge.first + output_offset << ' ' << edge.second + output_offset << '\n';
    }
    return true;
}

int main() {
    cout << "Podaj numer grafu (np. 3 dla graph3.txt) lub wcisnij Enter dla domyslnego: " << flush;
    string choice;
    getline(cin, choice);

    string input_name = "graph.txt";
    string output_name = "graph_out.txt";
    if (!choice.empty()) {
        input_name = "graph" + choice + ".txt";
        output_name = "graph_out" + choice + ".txt";
    }

    Graph graph;
    if (!load_graph(input_name, graph)) {
        cout << "Nie udalo sie wczytac pliku z grafem.\n";
        return 0;
    }

    cout << "Wczytano graf z pliku " << input_name << '\n';
    print_graph(graph);

    Graph original_graph;
    const bool is_conjugated = is_conjugated_graph(graph, original_graph);
    cout << "Graf jest sprzezony: " << boolalpha << is_conjugated << '\n';
    if (!is_conjugated) {
        return 0;
    }

    const bool is_line = is_line_graph(graph);
    cout << "Graf jest liniowy: " << boolalpha << is_line << '\n';

    if (save_graph(original_graph, output_name, graph.index_offset)) {
        cout << "Graf oryginalny zapisany do pliku " << output_name << '\n';
    } else {
        cout << "Nie udalo sie zapisac grafu do pliku.\n";
    }

    return 0;
}
