// Program heurystyczny do znajdowania lokalnego dopasowania wielu sekwencji (motywow).
// Cel: znalezienie wspolnych k-merow w roznych sekwencjach po filtracji jakosci.
// Algorytm:
// 1) filtruje sekwencje po progu jakosci,
// 2) tworzy wierzcholki dla wszystkich k-merow,
// 3) grupuje identyczne k-mery,
// 4) dla kazdej grupy buduje klike zachlannie,
// 5) wybiera najlepszy wynik.
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdlib> // atoi

using namespace std;

// Dane jednej sekwencji: identyfikator, sekwencja i oceny jakosci.
struct Sequence
{
    string id;
    string data;
    vector<int> quality;
};

// Wierzcholek grafu: jedno wystapienie k-meru w konkretnej sekwencji.
struct Node
{
    int seq_index;         // indeks sekwencji w wektorze
    int position_original; // pozycja startu k-meru w oryginalnej sekwencji (1-based)
    string chunk;          // k-mer (podciag dlugosci k)
};

// Wczytuje FASTA do list identyfikatorow i sekwencji.
bool read_fasta(const string& path, vector<string>& ids, vector<string>& sequences)
{
    ifstream file(path);
    if (!file.is_open())
    {
        cerr << "Nie udalo sie otworzyc pliku fasta: " << path << "\n";
        return false;
    }

    string line;
    string current_id;
    string current_sequence;

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        if (line[0] == '>')
        {
            if (!current_id.empty())
            {
                ids.push_back(current_id);
                sequences.push_back(current_sequence);
            }
            current_id = line.substr(1);
            current_sequence.clear();
        }
        else
        {
            for (char c : line)
            {
                if (!isspace(static_cast<unsigned char>(c)))
                    current_sequence.push_back(c);
            }
        }
    }

    if (!current_id.empty())
    {
        ids.push_back(current_id);
        sequences.push_back(current_sequence);
    }

    return true;
}

// Wczytuje QUAL do list identyfikatorow i list ocen jakosci.
bool read_qual(const string& path, vector<string>& ids, vector<vector<int>>& qualities)
{
    ifstream file(path);
    if (!file.is_open())
    {
        cerr << "Nie udalo sie otworzyc pliku qual: " << path << "\n";
        return false;
    }

    string line;
    string current_id;
    vector<int> current_quality;

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        if (line[0] == '>')
        {
            if (!current_id.empty())
            {
                ids.push_back(current_id);
                qualities.push_back(current_quality);
            }
            current_id = line.substr(1);
            current_quality.clear();
        }
        else
        {
            istringstream iss(line);
            int value = 0;
            while (iss >> value)
                current_quality.push_back(value);
        }
    }

    if (!current_id.empty())
    {
        ids.push_back(current_id);
        qualities.push_back(current_quality);
    }

    return true;
}

// Laduje FASTA+QUAL i sprawdza ich spojnosc.
bool load_sequences(const string& fasta_path, const string& qual_path, vector<Sequence>& sequences)
{
    vector<string> fasta_ids;
    vector<string> fasta_sequences;
    if (!read_fasta(fasta_path, fasta_ids, fasta_sequences))
        return false;

    vector<string> qual_ids;
    vector<vector<int>> qual_values;
    if (!read_qual(qual_path, qual_ids, qual_values))
        return false;

    if (fasta_ids.size() != qual_ids.size())
    {
        cerr << "Liczba sekwencji w plikach fasta i qual nie jest taka sama\n";
        return false;
    }

    for (size_t i = 0; i < fasta_ids.size(); i++)
    {
        if (fasta_ids[i] != qual_ids[i])
        {
            cerr << "Nie zgadza sie kolejnosc identyfikatorow: " << fasta_ids[i] << " vs " << qual_ids[i] << "\n";
            return false;
        }
        if (fasta_sequences[i].size() != qual_values[i].size())
        {
            cerr << "Dlugosc sekwencji i listy jakosci nie jest taka sama dla " << fasta_ids[i] << "\n";
            return false;
        }

        Sequence seq;
        seq.id = fasta_ids[i];
        seq.data = fasta_sequences[i];
        seq.quality = qual_values[i];
        sequences.push_back(seq);
    }

    return true;
}

// Filtruje sekwencje wedlug progu jakosci i zapisuje pozycje oryginalne.
void filter_sequence(const Sequence& input, int threshold, string& filtered, vector<int>& original_positions)
{
    for (size_t i = 0; i < input.data.size(); i++)
    {
        if (input.quality[i] >= threshold)
        {
            filtered.push_back(input.data[i]);
            original_positions.push_back(static_cast<int>(i) + 1);
        }
    }
}

// Liczy, ile roznych sekwencji wystepuje w danej grupie wierzcholkow.
size_t count_unique_sequences(const vector<Node>& nodes,
                              const vector<int>& group_nodes,
                              int seq_count)
{
    vector<bool> seen(seq_count, false);
    size_t count = 0;
    for (int idx : group_nodes)
    {
        int seq_idx = nodes[idx].seq_index;
        if (!seen[seq_idx])
        {
            seen[seq_idx] = true;
            count += 1;
        }
    }
    return count;
}

// Sprawdza, czy dwa wierzcholki moga byc polaczone krawedzia.
static bool are_compatible(const Node& left, const Node& right, int distance_limit)
{
    if (left.seq_index == right.seq_index)
        return false;
    return std::abs(left.position_original - right.position_original) <= distance_limit;
}

// Buduje klike zachlannie w ramach jednej grupy identycznych k-merow.
vector<int> greedy_maximal_clique_in_group(const vector<Node>& nodes,
                                           const vector<int>& group_nodes,
                                           int seq_count,
                                           int distance_limit)
{
    if (group_nodes.empty())
        return {};

    // Liczymy stopnie (ile kompatybilnych sasiedztw ma kazdy wierzcholek).
    vector<int> degrees(nodes.size(), 0);
    for (size_t i = 0; i < group_nodes.size(); ++i)
    {
        int idx_i = group_nodes[i];
        for (size_t j = i + 1; j < group_nodes.size(); ++j)
        {
            int idx_j = group_nodes[j];
            if (are_compatible(nodes[idx_i], nodes[idx_j], distance_limit))
            {
                degrees[idx_i] += 1;
                degrees[idx_j] += 1;
            }
        }
    }

    // Sortujemy wierzcholki: najpierw te z najwyzszym stopniem.
    vector<int> ordered = group_nodes;
    sort(ordered.begin(), ordered.end(),
         [&](int a, int b)
         {
             if (degrees[a] != degrees[b])
                 return degrees[a] > degrees[b];
             return nodes[a].position_original < nodes[b].position_original;
         });

    // Zachlannie budujemy klike z ograniczeniem: max 1 wierzcholek na sekwencje.
    vector<int> clique;
    vector<bool> used_seq(seq_count, false);
    for (int idx : ordered)
    {
        int seq_idx = nodes[idx].seq_index;
        if (used_seq[seq_idx])
            continue;
        bool ok = true;
        for (int chosen : clique)
        {
            if (!are_compatible(nodes[idx], nodes[chosen], distance_limit))
            {
                ok = false;
                break;
            }
        }
        if (ok)
        {
            clique.push_back(idx);
            used_seq[seq_idx] = true;
        }
    }

    return clique;
}

int main(int argc, char* argv[])
{
    // === Krok 1: Wczytanie i walidacja argumentow ===
    if (argc != 5)
    {
        cerr << "Uzycie: " << argv[0] << " <plik_fasta> <plik_qual> <prog_wiarygodnosci> <dlugosc_podciagu>\n";
        return 1;
    }

    string fasta_path = argv[1];
    string qual_path = argv[2];
    int threshold = atoi(argv[3]);
    int substring_length = atoi(argv[4]);

    if (threshold < 0)
    {
        cerr << "Prog wiarygodnosci musi byc nieujemny.\n";
        return 1;
    }
    if (substring_length < 4 || substring_length > 9)
    {
        cerr << "Dlugosc podciagu musi byc w zakresie 4-9.\n";
        return 1;
    }

    // === Krok 2: Wczytanie danych ===
    vector<Sequence> sequences;
    if (!load_sequences(fasta_path, qual_path, sequences))
        return 1;

    // === Krok 3: Filtracja po progu jakosci ===
    vector<string> filtered_data(sequences.size());
    vector<vector<int>> filtered_positions(sequences.size());
    for (size_t i = 0; i < sequences.size(); i++)
        filter_sequence(sequences[i], threshold, filtered_data[i], filtered_positions[i]);

    // Podglad przefiltrowanych sekwencji.
    cout << "[po usunieciu nukleotydow ponizej progu]\n";
    for (size_t i = 0; i < sequences.size(); i++)
    {
        cout << "seq" << (i + 1) << ":\n";
        cout << filtered_data[i] << "\n";
    }

    // === Krok 4: Generowanie wierzcholkow (k-mery) ===
    vector<Node> nodes;
    for (size_t i = 0; i < sequences.size(); i++)
    {
        if (filtered_data[i].size() < static_cast<size_t>(substring_length))
            continue;

        for (size_t start = 0; start + substring_length <= filtered_data[i].size(); start++)
        {
            Node node;
            node.seq_index = static_cast<int>(i);
            node.position_original = filtered_positions[i][start];
            node.chunk = filtered_data[i].substr(start, substring_length);
            nodes.push_back(node);
        }
    }

    if (nodes.empty())
    {
        cout << "Nie wygenerowano zadnych wierzcholkow. Sprobuj obnizyc prog wiarygodnosci.\n";
        return 0;
    }

    // === Krok 5: Grupowanie po identycznych k-merach ===
    map<string, vector<int>> groups;
    for (size_t i = 0; i < nodes.size(); i++)
        groups[nodes[i].chunk].push_back(static_cast<int>(i));

    int distance_limit = substring_length * 10;

    // === Krok 6: Szukanie najlepszej kliki ===
    vector<int> best_clique;
    size_t best_size = 0;
    int seq_count = static_cast<int>(sequences.size());

    for (const auto& entry : groups)
    {
        const vector<int>& group_nodes = entry.second;
        if (group_nodes.size() <= best_size)
            continue;

        if (count_unique_sequences(nodes, group_nodes, seq_count) <= best_size)
            continue;

        vector<int> clique = greedy_maximal_clique_in_group(nodes, group_nodes, seq_count, distance_limit);
        if (clique.size() > best_size)
        {
            best_clique = clique;
            best_size = clique.size();
            if (best_size == sequences.size())
                break;
        }
    }

    // === Krok 7: Wypisanie wyniku ===
    if (best_clique.empty())
    {
        cout << "Nie znaleziono zadnej kliki\n";
        return 0;
    }

    cout << "Znaleziono strukture o rozmiarze " << best_clique.size() << ":\n";
    for (int idx : best_clique)
    {
        const Node& node = nodes[idx];
        cout << "Sekwencja " << (node.seq_index + 1)
             << " (" << sequences[node.seq_index].id << "), pozycja "
             << node.position_original << ", fragment: "
             << node.chunk << "\n";
    }

    return 0;
}
