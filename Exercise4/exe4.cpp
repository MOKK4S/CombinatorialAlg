#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

using Counts = map<int, int>;
struct Stats {
    long long nodes = 0;
    long long candidates = 0;
};

static void restore(Counts& d, const vector<int>& removed) {
    for (int x : removed) d[x]++;
}

static bool takeDistances(int p, const vector<int>& points, Counts& d, vector<int>& removed) {
    removed.clear();
    for (int x : points) {
        int dist = abs(p - x);
        auto it = d.find(dist);
        if (it == d.end()) {
            restore(d, removed);
            return false;
        }
        if (--it->second == 0) d.erase(it);
        removed.push_back(dist);
    }
    return true;
}

static bool solve(Counts& d, vector<int>& points, int L, Stats& stats) {
    stats.nodes++;
    if (d.empty()) return true;
    int y = d.rbegin()->first;
    vector<int> removed;

    stats.candidates++;
    if (takeDistances(y, points, d, removed)) {
        points.push_back(y);
        if (solve(d, points, L, stats)) return true;
        points.pop_back();
        restore(d, removed);
    }

    int p = L - y;
    if (p != y) {
        stats.candidates++;
        if (takeDistances(p, points, d, removed)) {
        points.push_back(p);
        if (solve(d, points, L, stats)) return true;
        points.pop_back();
        restore(d, removed);
        }
    }
    return false;
}

static Counts buildCounts(const vector<int>& points) {
    Counts c;
    for (size_t i = 0; i < points.size(); ++i) {
        for (size_t j = i + 1; j < points.size(); ++j) {
            c[points[j] - points[i]]++;
        }
    }
    return c;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Uzycie: " << argv[0] << " <plik>\n";
        return 1;
    }

    ifstream in(argv[1]);
    if (!in) {
        cerr << "Nie mozna otworzyc pliku.\n";
        return 1;
    }

    vector<int> vals;
    int v;
    while (in >> v) vals.push_back(v);
    if (vals.empty()) {
        cerr << "Brak danych.\n";
        return 1;
    }

    for (int x : vals) {
        if (x <= 0) {
            cerr << "Niepoprawna wartosc: " << x << "\n";
            return 1;
        }
    }

    long long n = (long long)vals.size();
    long long disc = 1 + 8 * n;
    long long s = sqrt((long double)disc);
    while (s * s < disc) s++;
    while (s * s > disc) s--;
    if (s * s != disc || (1 + s) % 2 != 0) {
        cerr << "Nieprawidlowa liczba elementow: " << n << "\n";
        return 1;
    }

    int L = *max_element(vals.begin(), vals.end());
    Counts d, orig;
    for (int x : vals) {
        d[x]++;
        orig[x]++;
    }
    if (d[L] != 1) {
        cerr << "Nieprawidlowa instancja: brak jednoznacznej dlugosci.\n";
        return 1;
    }
    if (--d[L] == 0) d.erase(L);

    vector<int> points = {0, L};
    Stats stats;
    auto start = chrono::steady_clock::now();
    bool ok = solve(d, points, L, stats);
    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cerr << "Czas: " << ms << " ms\n";
    if (!ok) {
        cerr << "Nie znaleziono rozwiazania dla podanych elementow.\n";
        cout << "Zgodnosc mapy: NIE\n";
        cout << "Liczba odwiedzonych wezlow: " << stats.nodes << "\n";
        cout << "Liczba sprawdzonych kandydatow: " << stats.candidates << "\n";
        return 1;
    }

    sort(points.begin(), points.end());
    vector<int> segs;
    for (size_t i = 1; i < points.size(); ++i) segs.push_back(points[i] - points[i - 1]);

    cout << "Dlugosc: " << L << "\n";
    cout << "Mapa: ";
    for (size_t i = 0; i < segs.size(); ++i) {
        if (i) cout << " ";
        cout << segs[i];
    }
    cout << "\n";
    Counts built = buildCounts(points);
    cout << "Zgodnosc mapy: " << (built == orig ? "TAK" : "NIE") << "\n";
    cout << "Liczba odwiedzonych wezlow: " << stats.nodes << "\n";
    cout << "Liczba sprawdzonych kandydatow: " << stats.candidates << "\n";
    return 0;
}
