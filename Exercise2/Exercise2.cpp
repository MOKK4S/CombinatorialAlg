#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

struct Graph {
    size_t node_count;
    size_t edge_count;
    vector<pair<size_t, size_t>> edges;
};

Graph load_graph() {
    ifstream graph_file("Exercise2/graph.txt");
    if (!graph_file.is_open()) {
        graph_file.open("graph.txt");
    }

    Graph graph{};
    graph_file >> graph.node_count >> graph.edge_count;

    graph.edges.reserve(graph.edge_count);
    for (size_t i = 0; i < graph.edge_count; ++i) {
        size_t u;
        size_t v;
        graph_file >> u >> v;
        graph.edges.emplace_back(u, v);
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

int main() {
    Graph graph = load_graph();

    cout << graph.node_count << ' ' << graph.edge_count << '\n';
    for (const auto &edge : graph.edges) {
        cout << edge.first << ' ' << edge.second << '\n';
    }

    export_graph(graph);
    return 0;

    
}
