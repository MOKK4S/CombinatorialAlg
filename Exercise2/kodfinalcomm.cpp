// Program pomocniczy do sprawdzania, czy zadany graf skierowany
// jest grafem sprzężonym (G-grafem) oraz czy jest liniowy.
// Kod to wersja "komentowana" pliku razin.cpp.
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>

using namespace std;

// Odczytuje graf z pliku w formacie:
// N M
// u1 v1
// ...
// uM vM
// Numeracja może być 0- lub 1-indeksowa; wnętrze jest 0-indeksowe.
bool load_graph(const string& path, vector<vector<int>>& graph)
{
    ifstream file(path);
    if (!file.is_open())
    {
        cerr << "Nie udalo sie otworzyc pliku: " << path << "\n";
        return false;
    }

    int node_count = 0;
    int edge_count = 0;
    file >> node_count >> edge_count;
    if (!file)
    {
        cerr << "Niepoprawny naglowek w pliku: " << path << "\n";
        return false;
    }

    vector<pair<int, int>> edges;
    edges.reserve(edge_count);
    bool uses_zero_index = false;

    // Wczytanie łuków linia po linii; linie z samym numerem wierzchołka
    // traktujemy jako "brak krawędzi wychodzących" (pomocne przy grafach z wierzchołkami izolowanymi).
    string line;
    while (getline(file, line))
    {
        if (line.empty()) continue;
        istringstream iss(line);

        int from = 0;
        int to = 0;

        if (!(iss >> from))
            continue; // pusta linia po trimie

        // Jeśli nie ma drugiej liczby, zostawiamy wierzchołek bez krawędzi.
        if (!(iss >> to))
            continue;

        if (from == 0 || to == 0)
            uses_zero_index = true;

        edges.push_back({ from, to });
    }

    // Normalizacja indeksów do 0-bazowych.
    const int offset = uses_zero_index ? 0 : 1;
    graph.assign(node_count, {});

    // Przepisanie łuków do list sąsiedztwa.
    for (const auto& edge : edges)
    {
        const int from = edge.first - offset;
        const int to = edge.second - offset;

        if (from < 0 || to < 0 || from >= node_count || to >= node_count)
        {
            cerr << "Wykryto luk wykraczajacy poza zakres w pliku: " << path << "\n";
            continue;
        }
        graph[from].push_back(to);
    }

    return true;
}

// Zapisuje graf w tym samym formacie co wejście:
// najpierw nagłówek N M, potem M linii "u v" (1-indeksowo).
bool save_graph(const vector<vector<int>>& graph, const string& path)
{
    ofstream file(path);
    if (!file.is_open())
    {
        cerr << "Nie udalo sie zapisac pliku: " << path << "\n";
        return false;
    }

    long long node_count = static_cast<long long>(graph.size());
    long long edge_count = 0;
    for (const auto& neighbors : graph)
        edge_count += static_cast<long long>(neighbors.size());

    file << node_count << " " << edge_count << "\n";

    for (int from = 0; from < graph.size(); from++)
    {
        for (int to : graph[from])
            file << (from + 1) << " " << (to + 1) << "\n";
    }

    file.close();
    return true;
}

// Sprawdza własność G-grafu:
// żadne dwa różne wierzchołki nie mogą mieć wspólnych następców,
// chyba że ich listy następców są identyczne.
bool is_G_graph(const vector<vector<int>>& graph)
{
    for (int i = 0; i < graph.size(); i++)
    {
        vector<int> neighbors_i = graph[i];
        sort(neighbors_i.begin(), neighbors_i.end());

        for (int j = i + 1; j < graph.size(); j++)
        {
            vector<int> neighbors_j = graph[j];
            sort(neighbors_j.begin(), neighbors_j.end());

            vector<int> intersection;
            set_intersection
            (
                neighbors_i.begin(), neighbors_i.end(),
                neighbors_j.begin(), neighbors_j.end(),
                back_inserter(intersection)
            );

            if (!intersection.empty() && neighbors_i != neighbors_j)
                return false;
        }
    }
    return true;
}

// Buduje listę poprzedników: predecessors[v] = wierzchołki z krawędzią do v.
vector<vector<int>> build_predecessors(const vector<vector<int>>& graph)
{
    int node_count = graph.size();
    vector<vector<int>> predecessors(node_count);

    for (int i = 0; i < node_count; i++)
    {
        for (int neighbor : graph[i])
            predecessors[neighbor].push_back(i);
    }

    return predecessors;
}

// Weryfikuje, czy graf jest liniowy:
// jeśli dwa wierzchołki mają wspólnych następców,
// to muszą mieć identyczne listy następców i brak wspólnych poprzedników.
bool is_line_graph(const vector<vector<int>>& graph)
{
    vector<vector<int>> predecessor_graph = build_predecessors(graph);

    for (int i = 0; i < graph.size(); i++)
    {
        vector<int> neighbors_i = graph[i];
        sort(neighbors_i.begin(), neighbors_i.end());

        vector<int> predecessors_i = predecessor_graph[i];
        sort(predecessors_i.begin(), predecessors_i.end());

        for (int j = i + 1; j < graph.size(); j++)
        {
            vector<int> neighbors_j = graph[j];
            sort(neighbors_j.begin(), neighbors_j.end());

            vector<int> predecessors_j = predecessor_graph[j];
            sort(predecessors_j.begin(), predecessors_j.end());

            vector<int> intersection;
            set_intersection(
                neighbors_i.begin(), neighbors_i.end(),
                neighbors_j.begin(), neighbors_j.end(),
                back_inserter(intersection)
            );

            vector<int> predecessor_intersection;
            set_intersection(
                predecessors_i.begin(), predecessors_i.end(),
                predecessors_j.begin(), predecessors_j.end(),
                back_inserter(predecessor_intersection)
            );

            if (!intersection.empty() && (neighbors_i != neighbors_j || !predecessor_intersection.empty()))
                return false;
        }
    }
    return true;
}

// Wykrywa krawędzie wielokrotne (ten sam łuk zapisany więcej niż raz).
bool has_multiple_edges(const vector<vector<int>>& graph)
{
    for (const auto& neighbors : graph)
    {
        vector<int> sorted = neighbors;
        sort(sorted.begin(), sorted.end());
        for (size_t i = 1; i < sorted.size(); i++)
        {
            if (sorted[i] == sorted[i - 1])
                return true;
        }
    }
    return false;
}

// Tworzy łuki grafu H odpowiadającego grafowi G:
// każdy wierzchołek i ma parę (2*i, 2*i+1).
// Następnie "sklejamy" końce według krawędzi G.
vector<pair<int, int>> build_H_edges(const vector<vector<int>>& graph)
{
    int node_count = graph.size();
    vector<pair<int, int>> edges;

    for (int i = 0; i < node_count; i++)
        edges.push_back({ 2 * i, 2 * i + 1 });

    for (int i = 0; i < node_count; i++)
    {
        for (int neighbor : graph[i])
        {
            int source = edges[i].second;
            int target = edges[neighbor].first;

            for (auto& edge : edges)
            {
                if (edge.first == target) edge.first = source;
                if (edge.second == target) edge.second = source;
            }
        }
    }

    return edges;
}

// Konwertuje listę łuków (wierzchołki mogą być nieciągłe) do
// zwartej listy sąsiedztwa z ponownym indeksowaniem od 0.
vector<vector<int>> build_adjacency_list(const vector<pair<int, int>>& edges)
{
    vector<int> all_nodes;

    for (size_t i = 0; i < edges.size(); i++)
    {
        all_nodes.push_back(edges[i].first);
        all_nodes.push_back(edges[i].second);
    }

    sort(all_nodes.begin(), all_nodes.end());
    all_nodes.erase(unique(all_nodes.begin(), all_nodes.end()), all_nodes.end());

    map<int, int> reindex;
    for (size_t i = 0; i < all_nodes.size(); i++)
        reindex[all_nodes[i]] = static_cast<int>(i);

    vector<vector<int>> adjacency(all_nodes.size());

    for (size_t i = 0; i < edges.size(); i++)
    {
        int from = edges[i].first;
        int to = edges[i].second;
        int mapped_from = reindex[from];
        int mapped_to = reindex[to];

        adjacency[mapped_from].push_back(mapped_to);
    }

    return adjacency;
}

int main()
{
    // Wybór numeru grafu (graphX.txt) lub domyślnego graph.txt.
    cout << "Podaj numer grafu (np. 3 dla graph3.txt) lub wcisnij Enter dla domyslnego: " << flush;
    string choice;
    getline(cin, choice);

    string input_name = "graph.txt";
    string output_name = "graph_Output.txt";

    if (!choice.empty())
    {
        input_name = "graph" + choice + ".txt";
        output_name = "graph_Output" + choice + ".txt";
    }

    // Wczytanie grafu.
    vector<vector<int>> graph;
    if (!load_graph(input_name, graph))
        return 0;

    cout << "Wczytano graf z pliku " << input_name << endl;
    save_graph(graph, output_name); // Kopia wejścia w formacie wyjściowym.

    if (has_multiple_edges(graph))
        cout << "uwaga: graf zawiera krawedzie wielokrotne (multigraf)" << endl;

    if (is_G_graph(graph))
    {
        cout << "jest to graf sprzezony" << endl;

        if (is_line_graph(graph))
            cout << "jest to graf liniowy" << endl;
        else
            cout << "nie jest to graf liniowy" << endl;

        // Budowa grafu H i nadpisanie wyjścia.
        vector<pair<int, int>> edges = build_H_edges(graph);
        vector<vector<int>> graph_H = build_adjacency_list(edges);
        save_graph(graph_H, output_name);
    }
    else
    {
        cout << "ten graf nie jest sprzezony" << endl;
    }

    return 0;
}
