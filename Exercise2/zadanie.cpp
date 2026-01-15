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

void trim_text(string &text) {
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t' || text.front() == '\n' ||
                             text.front() == '\r')) {
        text.erase(text.begin());
    }
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\n' ||
                             text.back() == '\r')) {
        text.pop_back();
    }
}

bool read_graph_stream(istream &stream, Graph &graph) {
    size_t node_count = 0;
    size_t edge_count = 0;
    stream >> node_count >> edge_count;
    if (!stream) {
        return false;
    }

    vector<pair<size_t, size_t>> raw_edges;
    raw_edges.reserve(edge_count);
    bool uses_zero_index = false;

    for (size_t i = 0; i < edge_count; ++i) {
        size_t u = 0;
        size_t v = 0;
        stream >> u >> v;
        if (!stream) {
            return false;
        }
        raw_edges.emplace_back(u, v);
        if (u == 0 || v == 0) {
            uses_zero_index = true;
        }
    }

    const size_t index_offset = uses_zero_index ? 0 : 1;
    vector<pair<size_t, size_t>> edges;
    edges.reserve(raw_edges.size());

    for (const auto &edge : raw_edges) {
        size_t u = edge.first;
        size_t v = edge.second;
        if (u < index_offset || v < index_offset) {
            edges.emplace_back(u, v);
            continue;
        }
        u -= index_offset;
        v -= index_offset;
        edges.emplace_back(u, v);
    }

    graph.node_count = node_count;
    graph.index_offset = index_offset;
    graph.edges = edges;
    return true;
}



bool load_graph(const vector<string> &paths, Graph &graph, string &used_path) {
    for (const auto &path : paths) {
        ifstream file(path);
        if (!file.is_open()) {
            continue;
        }
        if (read_graph_stream(file, graph)) {
            used_path = path;
            return true;
        }
    }
    return false;
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



struct DisjointSet {
    explicit DisjointSet(size_t size) : parent(size) {
        for (size_t i = 0; i < parent.size(); ++i) {
            parent[i] = i;
        }
    }

    size_t find(size_t value) {
        if (parent[value] == value) {
            return value;
        }
        parent[value] = find(parent[value]);
        return parent[value];
    }

    void unite(size_t a, size_t b) {
        const size_t root_a = find(a);
        const size_t root_b = find(b);
        if (root_a == root_b) {
            return;
        }
        parent[root_b] = root_a;
    }

    vector<size_t> parent;
};

Graph build_original_candidate(const Graph &conjugated) {
    Graph original{};
    if (conjugated.node_count == 0) {
        return original;
    }

    const size_t endpoint_count = conjugated.node_count * 2;
    DisjointSet disjoint_set(endpoint_count);

    for (const auto &edge : conjugated.edges) {
        const size_t head_id = edge.first * 2 + 1;
        const size_t tail_id = edge.second * 2;
        if (head_id < endpoint_count && tail_id < endpoint_count) {
            disjoint_set.unite(head_id, tail_id);
        }
    }

    vector<size_t> root_to_id(endpoint_count, static_cast<size_t>(-1));
    size_t next_id = 0;
    vector<pair<size_t, size_t>> edges;
    edges.reserve(conjugated.node_count);

    for (size_t vertex = 0; vertex < conjugated.node_count; ++vertex) {
        const size_t tail_root = disjoint_set.find(vertex * 2);
        const size_t head_root = disjoint_set.find(vertex * 2 + 1);

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
    const size_t vertex_count = graph.edges.size();
    vector<pair<size_t, size_t>> edges;
    edges.reserve(vertex_count * 2);

    for (size_t i = 0; i < vertex_count; ++i) {
        for (size_t j = 0; j < vertex_count; ++j) {
            if (graph.edges[i].second == graph.edges[j].first) {
                edges.emplace_back(i, j);
            }
        }
    }

    line_graph.node_count = vertex_count;
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



bool recover_original_graph(const Graph &graph, Graph &original_graph) {
    if (!edges_in_range(graph)) {
        return false;
    }

    Graph candidate = build_original_candidate(graph);
    Graph reconstructed = build_line_graph(candidate);
    if (!graphs_equal(graph, reconstructed)) {
        return false;
    }

    original_graph = candidate;
    return true;
}

bool is_linear_graph(const Graph &graph) {
    if (!edges_in_range(graph)) {
        return false;
    }

    vector<size_t> degree(graph.node_count, 0);
    for (const auto &edge : graph.edges) {
        if (edge.first == edge.second) {
            degree[edge.first] += 2;
            if (degree[edge.first] > 2) {
                return false;
            }
            continue;
        }

        ++degree[edge.first];
        if (degree[edge.first] > 2) {
            return false;
        }
        ++degree[edge.second];
        if (degree[edge.second] > 2) {
            return false;
        }
    }

    return true;
}




bool save_graph(const Graph &graph,
                const vector<string> &paths,
                size_t output_offset,
                string &used_path) {
    for (const auto &path : paths) {
        ofstream file(path);
        if (!file.is_open()) {
            continue;
        }
        file << graph.node_count << ' ' << graph.edges.size() << '\n';
        for (const auto &edge : graph.edges) {
            file << edge.first + output_offset << ' ' << edge.second + output_offset << '\n';
        }
        used_path = path;
        return true;
    }
    return false;
}




int main() {
    cout << "Podaj numer grafu (np. 3 dla graph3.txt) lub wcisnij Enter dla domyslnego: " << flush;
    string choice;
    getline(cin, choice);
    trim_text(choice);

    string input_name = "graph.txt";
    string output_name = "graph_out.txt";

    if (!choice.empty()) {
        input_name = "graph" + choice + ".txt";
        output_name = "graph_out" + choice + ".txt";
    }

    vector<string> input_paths;
    input_paths.push_back(input_name);
    input_paths.push_back("Exercise2/" + input_name);

    Graph graph{};
    string used_input;
    if (!load_graph(input_paths, graph, used_input)) {
        cout << "Nie udalo sie wczytac pliku z grafem.\n";
        return 0;
    }

    cout << "Wczytano graf z pliku " << used_input << '\n';
    print_graph(graph);

    Graph original_graph;
    const bool conjugated = recover_original_graph(graph, original_graph);
    cout << "Graf jest sprzezony: " << boolalpha << conjugated << '\n';
    if (!conjugated) {
        return 0;
    }

    const bool linear = is_linear_graph(original_graph);
    cout << "Oryginalny graf jest liniowy: " << boolalpha << linear << '\n';

    vector<string> output_paths;
    output_paths.push_back(output_name);
    output_paths.push_back("Exercise2/" + output_name);

    string used_output;
    if (!save_graph(original_graph, output_paths, graph.index_offset, used_output)) {
        cout << "Nie udalo sie zapisac grafu.\n";
        return 0;
    }

    cout << "Graf oryginalny zapisany do pliku " << used_output << '\n';
    return 0;
}
