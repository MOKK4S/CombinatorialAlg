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

struct Graph {
    size_t node_count{};
    size_t index_offset{};
    vector<pair<size_t, size_t>> edges;
    vector<vector<size_t>> out_neighbors;
    vector<vector<size_t>> in_neighbors;
};

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

class DisjointSet {
public:
    explicit DisjointSet(size_t size) : parent(size) {
        iota(parent.begin(), parent.end(), 0);
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

    for (const auto &edge : conjugated.edges) {
        const size_t head_id = head_ids[edge.first];
        const size_t tail_id = tail_ids[edge.second];
        disjoint_set.unite(head_id, tail_id);
    }

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

    vector<pair<size_t, size_t>> edges;
    edges.reserve(conjugated.node_count);
    for (size_t vertex = 0; vertex < conjugated.node_count; ++vertex) {
        const size_t tail = map_endpoint(tail_ids[vertex]);
        const size_t head = map_endpoint(head_ids[vertex]);
        edges.emplace_back(tail, head);
    }

    return make_graph(next_id, 0, std::move(edges));
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

    return make_graph(vertex_count, 0, std::move(edges));
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
    original_graph = std::move(candidate);
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
        string resolved_input;
        Graph graph = load_graph(input_candidates, resolved_input);
        cout << "Loaded graph from " << resolved_input << " with " << graph.node_count << " vertices and "
             << graph.edges.size() << " edges.\n";

        const bool one_graph = is_one_graph(graph);
        cout << "Graph is one-graph (no multiple edges): " << boolalpha << one_graph << '\n';

        Graph original_graph;
        const bool conjugated = recover_original_graph(graph, original_graph);
        cout << "Graph is conjugated: " << boolalpha << conjugated << '\n';
        if (!conjugated) {
            return 0;
        }

        const bool linear = is_linear_graph(original_graph);
        cout << "Original graph is linear: " << boolalpha << linear << '\n';

        const string resolved_output =
            save_graph_with_candidates(original_graph, output_candidates, graph.index_offset);
        cout << "Original graph saved to " << resolved_output << '\n';
    } catch (const exception &ex) {
        cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
