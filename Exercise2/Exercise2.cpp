#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

struct Graph {
    size_t node_count{};
    size_t edge_count{};
    size_t index_offset{};
    vector<pair<size_t, size_t>> edges;
    vector<vector<bool>> adjacency;
};

Graph load_graph() {
    ifstream graph_file("Exercise2/graph.txt");
    if (!graph_file.is_open()) {
        graph_file.open("graph.txt");
    }

    Graph graph{};
    graph_file >> graph.node_count >> graph.edge_count;

    graph.edges.reserve(graph.edge_count);

    bool uses_zero_index = false;
    for (size_t i = 0; i < graph.edge_count; ++i) {
        size_t u;
        size_t v;
        graph_file >> u >> v;
        graph.edges.emplace_back(u, v);

        if (u == 0 || v == 0) {
            uses_zero_index = true;
        }
    }

    graph.index_offset = uses_zero_index ? 0 : 1;

    graph.adjacency.assign(graph.node_count, vector<bool>(graph.node_count, false));
    for (const auto &edge : graph.edges) {
        if (edge.first < graph.index_offset || edge.second < graph.index_offset) {
            continue;
        }

        const size_t u = edge.first - graph.index_offset;
        const size_t v = edge.second - graph.index_offset;

        if (u < graph.node_count && v < graph.node_count) {
            graph.adjacency[u][v] = true;
        }
    }

    return graph;
}

Graph export_graph(const Graph &graph) {
    ofstream graph_file("Exercise2/graph_out.txt");
    if (!graph_file.is_open()) {
        graph_file.open("graph_out.txt");
    }

    graph_file << graph.node_count << ' ' << graph.edge_count << '\n';
    for (const auto &edge : graph.edges) {
        graph_file << edge.first << ' ' << edge.second << '\n';
    }

    return graph;
}

bool is_conjected(const Graph &graph) {
    if (graph.node_count == 0) {
        return true;
    }

    const size_t index_offset = graph.index_offset;

    for (const auto &edge : graph.edges) {
        if (edge.first < index_offset || edge.second < index_offset) {
            return false;
        }

        const size_t u = edge.first - index_offset;
        const size_t v = edge.second - index_offset;

        if (u >= graph.node_count || v >= graph.node_count) {
            return false;
        }
    }

    if (graph.adjacency.empty()) {
        return true;
    }

    for (size_t i = 0; i < graph.node_count; ++i) {
        for (size_t j = 0; j < graph.node_count; ++j) {
            if (graph.adjacency[i][j] != graph.adjacency[j][i]) {
                return false;
            }
        }
    }

    return true;
}

bool is_linear(const Graph &graph) {
    if (graph.node_count == 0) {
        return true;
    }

    const size_t index_offset = graph.index_offset;

    vector<size_t> degree(graph.node_count, 0);

    for (const auto &edge : graph.edges) {
        if (edge.first < index_offset || edge.second < index_offset) {
            return false;
        }

        const size_t u = edge.first - index_offset;
        const size_t v = edge.second - index_offset;

        if (u >= graph.node_count || v >= graph.node_count) {
            return false;
        }

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

int main() {
    Graph graph = load_graph();

    cout << graph.node_count << ' ' << graph.edge_count << '\n';
    for (const auto &edge : graph.edges) {
        cout << edge.first << ' ' << edge.second << '\n';
    }

    const bool conjugated = is_conjected(graph);
    cout << "Graph is conjugated: " << boolalpha << conjugated << '\n';
    const bool linear = is_linear(graph);
    cout << "Graph is linear: " << boolalpha << linear << '\n';
    export_graph(graph);
    return 0;

    
}
