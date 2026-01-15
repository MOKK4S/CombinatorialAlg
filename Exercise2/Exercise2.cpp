#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct Graph {
    size_t node_count{};
    size_t index_offset{};
    vector<pair<size_t, size_t>> edges;
    vector<vector<size_t>> out_neighbors;
    vector<vector<size_t>> in_neighbors;
};

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

Graph make_graph(size_t node_count, size_t index_offset, vector<pair<size_t, size_t>> edges) {
    Graph graph{};
    graph.node_count = node_count;
    graph.index_offset = index_offset;
    graph.edges = edges;
    graph.out_neighbors.clear();
    graph.in_neighbors.clear();
    graph.out_neighbors.resize(node_count);
    graph.in_neighbors.resize(node_count);

    for (size_t i = 0; i < graph.edges.size(); ++i) {
        const size_t from = graph.edges[i].first;
        const size_t to = graph.edges[i].second;
        graph.out_neighbors[from].push_back(to);
        graph.in_neighbors[to].push_back(from);
    }

    return graph;
}

class DisjointSet {
public:
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

private:
    vector<size_t> parent;
};

Graph parse_graph_stream(istream &graph_stream) {
    size_t node_count = 0;
    size_t edge_count = 0;
    graph_stream >> node_count >> edge_count;

    vector<pair<size_t, size_t>> edges;

    bool uses_zero_index = false;
    for (size_t i = 0; i < edge_count; ++i) {
        size_t u = 0;
        size_t v = 0;
        graph_stream >> u >> v;
        edges.push_back({u, v});
        if (u == 0 || v == 0) {
            uses_zero_index = true;
        }
    }

    const size_t index_offset = uses_zero_index ? 0 : 1;
    vector<pair<size_t, size_t>> normalized_edges;

    for (size_t i = 0; i < edges.size(); ++i) {
        const size_t u = edges[i].first - index_offset;
        const size_t v = edges[i].second - index_offset;
        normalized_edges.push_back({u, v});
    }

    return make_graph(node_count, index_offset, normalized_edges);
}

bool load_graph(const vector<string> &candidate_paths, Graph &graph, string &resolved_path) {
    for (const auto &path : candidate_paths) {
        ifstream graph_file(path);
        if (!graph_file.is_open()) {
            continue;
        }
        graph = parse_graph_stream(graph_file);
        resolved_path = path;
        return true;
    }
    return false;
}

bool save_graph(const Graph &graph, const string &output_path, size_t output_offset) {
    ofstream graph_file(output_path);
    if (!graph_file.is_open()) {
        return false;
    }

    graph_file << graph.node_count << ' ' << graph.edges.size() << '\n';
    for (const auto &edge : graph.edges) {
        graph_file << edge.first + output_offset << ' ' << edge.second + output_offset << '\n';
    }
    return true;
}

bool save_graph_with_candidates(const Graph &graph,
                                const vector<string> &candidate_paths,
                                size_t output_offset,
                                string &resolved_path) {
    for (const auto &path : candidate_paths) {
        if (save_graph(graph, path, output_offset)) {
            resolved_path = path;
            return true;
        }
    }
    return false;
}

Graph build_candidate_original(const Graph &conjugated) {
    if (conjugated.node_count == 0) {
        return make_graph(0, 0, {});
    }

    const size_t endpoint_count = conjugated.node_count * 2;
    DisjointSet disjoint_set(endpoint_count);

    vector<size_t> tail_ids(conjugated.node_count);
    vector<size_t> head_ids(conjugated.node_count);
    for (size_t vertex = 0; vertex < conjugated.node_count; ++vertex) {
        tail_ids[vertex] = vertex * 2;
        head_ids[vertex] = vertex * 2 + 1;
    }

    for (size_t i = 0; i < conjugated.edges.size(); ++i) {
        const size_t head_id = head_ids[conjugated.edges[i].first];
        const size_t tail_id = tail_ids[conjugated.edges[i].second];
        disjoint_set.unite(head_id, tail_id);
    }

    vector<size_t> root_to_id(endpoint_count, numeric_limits<size_t>::max());
    size_t next_id = 0;

    vector<pair<size_t, size_t>> edges;
    for (size_t vertex = 0; vertex < conjugated.node_count; ++vertex) {
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

    return make_graph(next_id, 0, edges);
}

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

    return make_graph(vertex_count, 0, edges);
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

bool recover_original_graph(const Graph &graph, Graph &original_graph) {
    Graph candidate = build_candidate_original(graph);
    Graph reconstructed = build_line_digraph(candidate);
    if (!graphs_equal(graph, reconstructed)) {
        return false;
    }
    original_graph = candidate;
    return true;
}

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
    vector<string> input_candidates;
    vector<string> output_candidates;

    string chosen_id;
    if (argc <= 1) {
        cout << "Enter graph number (e.g., 3 for graph3.txt) or press Enter for defaults: " << flush;
        string line;
        if (getline(cin, line)) {
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

    string resolved_input;
    Graph graph;
    if (!load_graph(input_candidates, graph, resolved_input)) {
        cout << "Unable to load graph.\n";
        return 0;
    }
    cout << "Loaded graph from " << resolved_input << " with " << graph.node_count << " vertices and "
         << graph.edges.size() << " edges.\n";

    Graph original_graph;
    const bool conjugated = recover_original_graph(graph, original_graph);
    cout << "Graph is conjugated: " << boolalpha << conjugated << '\n';
    if (!conjugated) {
        return 0;
    }

    const bool linear = is_linear_graph(original_graph);
    cout << "Original graph is linear: " << boolalpha << linear << '\n';

    string resolved_output;
    if (!save_graph_with_candidates(original_graph, output_candidates, graph.index_offset, resolved_output)) {
        cout << "Unable to save graph.\n";
        return 0;
    }
    cout << "Original graph saved to " << resolved_output << '\n';

    return 0;
}
