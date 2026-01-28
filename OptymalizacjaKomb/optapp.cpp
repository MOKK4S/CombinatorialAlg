#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

struct ScoreParams {
    int matchScore;
    int mismatchScore;
    int gapPenalty;
};

struct AcoParams {
    int ants;
    int iterations;
    double evaporation;
    double tau0;
    int maxFullMask;
};

struct AlignmentResult {
    vector<string> aligned;
    long long score;
    vector<string> edgeKeys;
};

// Przyciecie bialych znakow
string trim(const string &text) {
    size_t start = 0;
    while (start < text.size() && isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }
    size_t end = text.size();
    while (end > start && isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return text.substr(start, end - start);
}

// Czyszczenie sekwencji
string cleanSequence(const string &raw) {
    string out;
    out.reserve(raw.size());
    for (unsigned char ch : raw) {
        if (isalpha(ch)) {
            out.push_back(static_cast<char>(toupper(ch)));
        }
    }
    return out;
}

// Wczytanie sekwencji
bool loadSequencesFromFile(const string &path, vector<string> &names, vector<string> &sequences) {
    ifstream file(path);
    if (!file) {
        return false;
    }

    vector<string> lines;
    string line;
    while (getline(file, line)) {
        lines.push_back(line);
    }

    bool isFasta = false;
    for (const string &raw : lines) {
        string t = trim(raw);
        if (!t.empty() && t[0] == '>') {
            isFasta = true;
            break;
        }
    }

    names.clear();
    sequences.clear();

    if (isFasta) {
        string currentName;
        string currentSeq;
        for (const string &raw : lines) {
            string t = trim(raw);
            if (t.empty()) {
                continue;
            }
            if (t[0] == '>') {
                if (!currentName.empty()) {
                    string seq = cleanSequence(currentSeq);
                    if (!seq.empty()) {
                        names.push_back(currentName);
                        sequences.push_back(seq);
                    }
                }
                currentName = t.substr(1);
                currentSeq.clear();
            } else {
                currentSeq += cleanSequence(t);
            }
        }
        if (!currentName.empty()) {
            string seq = cleanSequence(currentSeq);
            if (!seq.empty()) {
                names.push_back(currentName);
                sequences.push_back(seq);
            }
        }
    } else {
        int index = 1;
        for (const string &raw : lines) {
            string t = trim(raw);
            if (t.empty()) {
                continue;
            }
            string seq = cleanSequence(t);
            if (!seq.empty()) {
                names.push_back("seq" + to_string(index++));
                sequences.push_back(seq);
            }
        }
    }

    return !sequences.empty();
}

// Zapis sekwencji do FASTA
bool saveSequencesToFasta(const string &path, const vector<string> &names,
                          const vector<string> &sequences) {
    ofstream file(path);
    if (!file) {
        return false;
    }

    for (size_t i = 0; i < sequences.size(); ++i) {
        string name = (i < names.size()) ? names[i] : ("seq" + to_string(i + 1));
        file << ">" << name << "\n";
        const string &seq = sequences[i];
        for (size_t pos = 0; pos < seq.size(); pos += 80) {
            file << seq.substr(pos, 80) << "\n";
        }
    }
    return true;
}

// Generator sekwencji
vector<string> generateRandomSequences(int count, int length, const string &alphabet, bool withMotif,
                                       int motifLength, int maxMutations, vector<string> &names,
                                       mt19937 &rng) {
    vector<string> sequences;
    sequences.reserve(count);
    names.clear();
    names.reserve(count);

    uniform_int_distribution<int> charDist(0, static_cast<int>(alphabet.size() - 1));
    string motif;

    if (withMotif && motifLength > 0 && motifLength <= length) {
        motif.resize(motifLength);
        for (int i = 0; i < motifLength; ++i) {
            motif[i] = alphabet[charDist(rng)];
        }
    }

    for (int i = 0; i < count; ++i) {
        string seq(length, 'A');
        for (int j = 0; j < length; ++j) {
            seq[j] = alphabet[charDist(rng)];
        }

        if (!motif.empty()) {
            int minStart = (length > motifLength) ? 1 : 0;
            int maxStart = max(minStart, length - motifLength);
            uniform_int_distribution<int> startDist(minStart, maxStart);
            int startPos = startDist(rng);

            string motifVariant = motif;
            uniform_int_distribution<int> mutCountDist(0, max(0, maxMutations));
            uniform_int_distribution<int> motifPosDist(0, motifLength - 1);
            int mutations = mutCountDist(rng);
            for (int m = 0; m < mutations; ++m) {
                int pos = motifPosDist(rng);
                char newChar = motifVariant[pos];
                while (newChar == motifVariant[pos]) {
                    newChar = alphabet[charDist(rng)];
                }
                motifVariant[pos] = newChar;
            }
            seq.replace(startPos, motifLength, motifVariant);
        }

        names.push_back("seq" + to_string(i + 1));
        sequences.push_back(seq);
    }

    return sequences;
}

// Ocena pary znakow
int scorePair(char a, char b, const ScoreParams &params) {
    if (a == '-' && b == '-') {
        return 0;
    }
    if (a == '-' || b == '-') {
        return params.gapPenalty;
    }
    if (a == b) {
        return params.matchScore;
    }
    return params.mismatchScore;
}

// Ocena kolumny
int scoreColumn(const vector<string> &sequences, const vector<int> &positions, uint64_t mask,
                const ScoreParams &params) {
    int score = 0;
    int count = static_cast<int>(sequences.size());
    for (int i = 0; i < count; ++i) {
        char ci = (mask & (1ULL << i)) ? sequences[i][positions[i]] : '-';
        for (int j = i + 1; j < count; ++j) {
            char cj = (mask & (1ULL << j)) ? sequences[j][positions[j]] : '-';
            score += scorePair(ci, cj, params);
        }
    }
    return score;
}

// Heurystyka ruchu
double heuristicValue(int columnScore) {
    double value = static_cast<double>(columnScore) / 4.0;
    value = max(-20.0, min(20.0, value));
    return exp(value);
}

// Klucz krawedzi
string makeEdgeKey(const vector<int> &positions, uint64_t mask) {
    string key;
    key.reserve(positions.size() * 4 + 24);
    for (size_t i = 0; i < positions.size(); ++i) {
        key += to_string(positions[i]);
        if (i + 1 < positions.size()) {
            key += ',';
        }
    }
    key += '|';
    key += to_string(mask);
    return key;
}

// Lista mozliwych ruchow
vector<uint64_t> generateMoveMasks(const vector<int> &positions, const vector<int> &lengths,
                                   int maxFullMask) {
    int count = static_cast<int>(positions.size());
    vector<int> active;
    for (int i = 0; i < count; ++i) {
        if (positions[i] < lengths[i]) {
            active.push_back(i);
        }
    }

    vector<uint64_t> masks;
    if (active.empty()) {
        return masks;
    }

    if (static_cast<int>(active.size()) <= maxFullMask) {
        uint64_t total = (active.size() >= 63) ? 0 : (1ULL << active.size());
        for (uint64_t localMask = 1; localMask < total; ++localMask) {
            uint64_t mask = 0;
            for (size_t bit = 0; bit < active.size(); ++bit) {
                if (localMask & (1ULL << bit)) {
                    mask |= (1ULL << active[bit]);
                }
            }
            masks.push_back(mask);
        }
    } else {
        for (int idx : active) {
            masks.push_back(1ULL << idx);
        }
        if (active.size() <= 12) {
            for (size_t i = 0; i < active.size(); ++i) {
                for (size_t j = i + 1; j < active.size(); ++j) {
                    masks.push_back((1ULL << active[i]) | (1ULL << active[j]));
                }
            }
        }
        uint64_t allMask = 0;
        for (int idx : active) {
            allMask |= (1ULL << idx);
        }
        masks.push_back(allMask);
        sort(masks.begin(), masks.end());
        masks.erase(unique(masks.begin(), masks.end()), masks.end());
    }

    return masks;
}

// Budowanie rozwiazania mrowki
AlignmentResult buildAntAlignment(const vector<string> &sequences, const vector<int> &lengths,
                                  const ScoreParams &scoring, const AcoParams &params,
                                  const unordered_map<string, double> &pheromone, mt19937 &rng) {
    int count = static_cast<int>(sequences.size());
    vector<int> positions(count, 0);
    vector<string> aligned(count);
    vector<string> edgeKeys;
    long long totalScore = 0;

    struct MoveOption {
        uint64_t mask;
        int columnScore;
        double desirability;
        string key;
    };

    while (true) {
        bool finished = true;
        for (int i = 0; i < count; ++i) {
            if (positions[i] < lengths[i]) {
                finished = false;
                break;
            }
        }
        if (finished) {
            break;
        }

        vector<uint64_t> masks = generateMoveMasks(positions, lengths, params.maxFullMask);
        vector<MoveOption> options;
        options.reserve(masks.size());

        for (uint64_t mask : masks) {
            string key = makeEdgeKey(positions, mask);
            int columnScore = scoreColumn(sequences, positions, mask, scoring);
            auto it = pheromone.find(key);
            double tau = (it == pheromone.end()) ? params.tau0 : it->second;
            double eta = heuristicValue(columnScore);
            double desirability = tau * eta;
            options.push_back({mask, columnScore, desirability, key});
        }

        double sum = 0.0;
        for (const auto &opt : options) {
            sum += opt.desirability;
        }

        size_t chosenIndex = 0;
        if (sum <= 0.0) {
            uniform_int_distribution<size_t> dist(0, options.size() - 1);
            chosenIndex = dist(rng);
        } else {
            uniform_real_distribution<double> dist(0.0, sum);
            double pick = dist(rng);
            double acc = 0.0;
            for (size_t i = 0; i < options.size(); ++i) {
                acc += options[i].desirability;
                if (pick <= acc) {
                    chosenIndex = i;
                    break;
                }
            }
        }

        const MoveOption &chosen = options[chosenIndex];
        edgeKeys.push_back(chosen.key);
        totalScore += chosen.columnScore;

        for (int i = 0; i < count; ++i) {
            if (chosen.mask & (1ULL << i)) {
                aligned[i].push_back(sequences[i][positions[i]]);
                positions[i]++;
            } else {
                aligned[i].push_back('-');
            }
        }
    }

    return {aligned, totalScore, edgeKeys};
}

// Uruchomienie ACO
AlignmentResult runAco(const vector<string> &sequences, const ScoreParams &scoring,
                       const AcoParams &params, unsigned int seed) {
    vector<int> lengths;
    lengths.reserve(sequences.size());
    for (const string &seq : sequences) {
        lengths.push_back(static_cast<int>(seq.size()));
    }

    unordered_map<string, double> pheromone;
    mt19937 rng(seed);
    AlignmentResult bestOverall;
    bestOverall.score = numeric_limits<long long>::min();

    for (int iter = 0; iter < params.iterations; ++iter) {
        long long bestScore = numeric_limits<long long>::min();
        AlignmentResult bestIter;

        for (int a = 0; a < params.ants; ++a) {
            AlignmentResult result = buildAntAlignment(sequences, lengths, scoring, params, pheromone, rng);
            if (result.score > bestScore) {
                bestScore = result.score;
                bestIter = result;
            }
            if (result.score > bestOverall.score) {
                bestOverall = std::move(result);
            }
        }

        for (auto &entry : pheromone) {
            entry.second *= (1.0 - params.evaporation);
        }

        for (const string &key : bestIter.edgeKeys) {
            pheromone[key] += 1.0;
        }
    }

    return bestOverall;
}

// Wejscie tekstowe
string readLine(const string &prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    return line;
}

// Wejscie liczby calkowitej
int readInt(const string &prompt) {
    while (true) {
        string line = readLine(prompt);
        string trimmed = trim(line);
        if (trimmed.empty()) {
            continue;
        }
        stringstream ss(trimmed);
        int value = 0;
        if (ss >> value && ss.eof()) {
            return value;
        }
        cout << "Nieprawidlowa wartosc.\n";
    }
}

// Wejscie liczby calkowitej z domyslna
int readIntWithDefault(const string &prompt, int defaultValue) {
    while (true) {
        cout << prompt << " [" << defaultValue << "]: ";
        string line;
        if (!getline(cin, line)) {
            return defaultValue;
        }
        string trimmed = trim(line);
        if (trimmed.empty()) {
            return defaultValue;
        }
        stringstream ss(trimmed);
        int value = 0;
        if (ss >> value && ss.eof()) {
            return value;
        }
        cout << "Nieprawidlowa wartosc.\n";
    }
}

// Wejscie liczby zmiennoprzecinkowej z domyslna
double readDoubleWithDefault(const string &prompt, double defaultValue) {
    while (true) {
        cout << prompt << " [" << defaultValue << "]: ";
        string line;
        if (!getline(cin, line)) {
            return defaultValue;
        }
        string trimmed = trim(line);
        if (trimmed.empty()) {
            return defaultValue;
        }
        stringstream ss(trimmed);
        double value = 0.0;
        if (ss >> value && ss.eof()) {
            return value;
        }
        cout << "Nieprawidlowa wartosc.\n";
    }
}

// Wejscie tak/nie
bool readYesNo(const string &prompt, bool defaultValue) {
    while (true) {
        cout << prompt << " [" << (defaultValue ? "t" : "n") << "]: ";
        string line;
        if (!getline(cin, line)) {
            return defaultValue;
        }
        string trimmed = trim(line);
        if (trimmed.empty()) {
            return defaultValue;
        }
        char c = static_cast<char>(tolower(static_cast<unsigned char>(trimmed[0])));
        if (c == 't' || c == 'y') {
            return true;
        }
        if (c == 'n') {
            return false;
        }
        cout << "Podaj t lub n.\n";
    }
}

// Wejscie ziarna
unsigned int readSeed(const string &prompt) {
    cout << prompt << " (puste = losowe): ";
    string line;
    if (!getline(cin, line)) {
        random_device rd;
        return rd();
    }
    string trimmed = trim(line);
    if (trimmed.empty()) {
        random_device rd;
        return rd();
    }
    stringstream ss(trimmed);
    unsigned int seed = 0;
    if (ss >> seed && ss.eof()) {
        return seed;
    }
    random_device rd;
    return rd();
}

// Wyswietlenie sekwencji
void printSequences(const vector<string> &names, const vector<string> &sequences) {
    if (sequences.empty()) {
        cout << "Brak sekwencji.\n";
        return;
    }
    for (size_t i = 0; i < sequences.size(); ++i) {
        string name = (i < names.size()) ? names[i] : ("seq" + to_string(i + 1));
        cout << "[" << (i + 1) << "] " << name << " (len=" << sequences[i].size() << ")\n";
        cout << sequences[i] << "\n";
    }
}

// Wyswietlenie dopasowania
void printAlignment(const vector<string> &names, const AlignmentResult &alignment) {
    for (size_t i = 0; i < alignment.aligned.size(); ++i) {
        string name = (i < names.size()) ? names[i] : ("seq" + to_string(i + 1));
        cout << name << "\n";
        cout << alignment.aligned[i] << "\n";
    }
}

// Menu glowne
void printMenu() {
    cout << "\n======== MSA + ACO ========\n";
    cout << "1. Wczytaj sekwencje z pliku\n";
    cout << "2. Wygeneruj sekwencje\n";
    cout << "3. Pokaz aktualne sekwencje\n";
    cout << "4. Uruchom ACO\n";
    cout << "5. Zapisz ostatnie dopasowanie\n";
    cout << "0. Wyjscie\n";
    cout << "\n===========================\n";
}

int main() {
    vector<string> names;
    vector<string> sequences;
    AlignmentResult lastAlignment;
    bool hasAlignment = false;

    while (true) {
        printMenu();
        int choice = readInt("Wybor: ");

        if (choice == 0) {
            break;
        }
        if (choice == 1) {
            // Wczytanie danych
            string path = readLine("Podaj sciezke do pliku: ");
            if (loadSequencesFromFile(path, names, sequences)) {
                cout << "Wczytano " << sequences.size() << " sekwencji.\n";
                hasAlignment = false;
            } else {
                cout << "Nie udalo sie wczytac pliku.\n";
            }
        } else if (choice == 2) {
            // Generator danych
            int count = readIntWithDefault("Liczba sekwencji", 5);
            int length = readIntWithDefault("Dlugosc sekwencji", 60);
            string alphabet = readLine("Alfabet (np. ACGT) [ACGT]: ");
            alphabet = cleanSequence(alphabet);
            if (alphabet.empty()) {
                alphabet = "ACGT";
            }
            bool withMotif = readYesNo("Wstawic wspolny motyw?", true);
            int motifLength = 0;
            int maxMutations = 0;
            if (withMotif) {
                motifLength = readIntWithDefault("Dlugosc motywu", 12);
                maxMutations = readIntWithDefault("Maks. liczba mutacji", 2);
            }
            unsigned int seed = readSeed("Ziarno generatora");
            mt19937 rng(seed);
            sequences = generateRandomSequences(count, length, alphabet, withMotif, motifLength, maxMutations,
                                                names, rng);
            cout << "Wygenerowano " << sequences.size() << " sekwencji.\n";
            hasAlignment = false;

            if (readYesNo("Zapisac do pliku FASTA?", true)) {
                string path = readLine("Sciezka do pliku: ");
                if (saveSequencesToFasta(path, names, sequences)) {
                    cout << "Zapisano do " << path << "\n";
                } else {
                    cout << "Nie udalo sie zapisac pliku.\n";
                }
            }
        } else if (choice == 3) {
            // Podglad sekwencji
            printSequences(names, sequences);
        } else if (choice == 4) {
            // Uruchomienie ACO
            if (sequences.empty()) {
                cout << "Brak sekwencji do dopasowania.\n";
                continue;
            }
            if (sequences.size() >= 63) {
                cout << "Zbyt wiele sekwencji dla reprezentacji maski.\n";
                continue;
            }

            ScoreParams scoring;
            scoring.matchScore = readIntWithDefault("Match score", 2);
            scoring.mismatchScore = readIntWithDefault("Mismatch score", -1);
            scoring.gapPenalty = readIntWithDefault("Gap penalty", -2);

            AcoParams params;
            params.ants = readIntWithDefault("Liczba mrowek", 30);
            params.iterations = readIntWithDefault("Liczba iteracji", 100);
            params.evaporation = readDoubleWithDefault("Parowanie (rho)", 0.3);
            params.tau0 = 1.0;
            params.maxFullMask = 10;

            unsigned int seed = readSeed("Ziarno ACO");
            auto start = chrono::steady_clock::now();
            AlignmentResult best = runAco(sequences, scoring, params, seed);
            auto end = chrono::steady_clock::now();
            chrono::duration<double> elapsed = end - start;

            cout << "Najlepszy wynik: " << best.score << "\n";
            if (!best.aligned.empty()) {
                cout << "Dlugosc dopasowania: " << best.aligned[0].size() << "\n";
            }
            cout << "Czas: " << elapsed.count() << " s\n";
            lastAlignment = best;
            hasAlignment = true;

            if (readYesNo("Wyswietlic dopasowanie?", true)) {
                printAlignment(names, best);
            }
            if (readYesNo("Zapisac dopasowanie do pliku?", false)) {
                string path = readLine("Sciezka do pliku: ");
                if (saveSequencesToFasta(path, names, best.aligned)) {
                    cout << "Zapisano do " << path << "\n";
                } else {
                    cout << "Nie udalo sie zapisac pliku.\n";
                }
            }
        } else if (choice == 5) {
            // Zapis dopasowania
            if (!hasAlignment) {
                cout << "Brak dopasowania do zapisu.\n";
                continue;
            }
            string path = readLine("Sciezka do pliku: ");
            if (saveSequencesToFasta(path, names, lastAlignment.aligned)) {
                cout << "Zapisano do " << path << "\n";
            } else {
                cout << "Nie udalo sie zapisac pliku.\n";
            }
        } else {
            cout << "Nieznana opcja.\n";
        }
    }

    return 0;
}
