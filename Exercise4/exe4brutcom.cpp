#include <algorithm> // algorytmy standardowe (sort, max_element, reverse)
#include <chrono> // narzedzia do pomiaru czasu
#include <cmath> // funkcje matematyczne (sqrt, abs)
#include <fstream> // obsluga plikow
#include <iostream> // wejscie/wyjscie standardowe
#include <map> // kontener map
#include <vector> // kontener wektor
using namespace std; // uzyj przestrzeni nazw std

using Counts = map<int, int>; // alias mapy zliczen odleglosci
struct Stats { // struktura statystyk przeszukiwania
    long long nodes = 0; // liczba odwiedzonych wezlow
    long long candidates = 0; // liczba sprawdzonych kandydatow
}; // koniec definicji Stats

static void restore(Counts& d, const vector<int>& removed) { // przywroc usuniete odleglosci do mapy
    for (int x : removed) d[x]++; // zwieksz licznik dla kazdej usunietej odleglosci
} // koniec restore

static bool takeDistances(int p, const vector<int>& points, Counts& d, vector<int>& removed) { // probuje zabrac odleglosci dla nowego punktu
    removed.clear(); // wyczysc liste usunietych odleglosci
    for (int x : points) { // przejdz po istniejacych punktach
        int dist = abs(p - x); // policz odleglosc do punktu
        auto it = d.find(dist); // znajdz odleglosc w mapie zliczen
        if (it == d.end()) { // brak wymaganej odleglosci
            restore(d, removed); // przywroc dotychczas usuniete odleglosci
            return false; // nie mozna dodac punktu
        } // koniec bloku bledu
        if (--it->second == 0) d.erase(it); // zmniejsz licznik i usun jesli zero
        removed.push_back(dist); // zapamietaj usunieta odleglosc
    } // koniec petli po punktach
    return true; // wszystkie odleglosci usuniete poprawnie
} // koniec takeDistances

static bool solve(Counts& d, vector<int>& points, int L, Stats& stats) { // glowna funkcja rekurencyjna
    stats.nodes++; // zlicz odwiedzony wezel
    if (d.empty()) return true; // jesli nie ma odleglosci, mamy rozwiazanie

    vector<int> candidate_points; // lista kandydatow na punkt
    for (auto const& [val, count] : d) { // przejdz po odleglosciach w mapie
        candidate_points.push_back(val); // dodaj wartosc jako kandydata
    } // koniec petli po mapie
    reverse(candidate_points.begin(), candidate_points.end()); // sprawdzaj kandydatow od najwiekszego

    for (int p : candidate_points) { // przejdz po kandydatach

        bool placed = false; // flaga czy punkt juz istnieje
        for (int pt : points) // sprawdz czy kandydat jest juz w punktach
            if (pt == p) placed = true; // ustaw flage gdy znajdziesz
        if (placed) continue; // pomin jesli punkt juz jest

        vector<int> removed; // lista usunietych odleglosci
        stats.candidates++; // zwieksz licznik kandydatow
        if (takeDistances(p, points, d, removed)) { // jesli da sie zabrac odleglosci
            points.push_back(p); // dodaj punkt do rozwiazania
            if (solve(d, points, L, stats)) return true; // rekurencyjnie szukaj dalej
            points.pop_back(); // cofniecie wyboru punktu
            restore(d, removed); // przywroc odleglosci po nieudanej probie
        } // koniec bloku proby umieszczenia
    } // koniec petli po kandydatach

    return false; // brak rozwiazania w tej galezi
} // koniec solve

static Counts buildCounts(const vector<int>& points) { // zbuduj mape odleglosci z punktow
    Counts c; // mapa zliczen
    for (size_t i = 0; i < points.size(); ++i) { // petla po pierwszym indeksie
        for (size_t j = i + 1; j < points.size(); ++j) { // petla po drugim indeksie
            c[points[j] - points[i]]++; // zwieksz zliczenie odleglosci
        } // koniec petli j
    } // koniec petli i
    return c; // zwroc mape zliczen
} // koniec buildCounts

int main(int argc, char** argv) { // punkt wejscia programu
    if (argc < 2) { // sprawdz czy podano sciezke do pliku
        cerr << "Uzycie: " << argv[0] << " <plik>\n"; // wypisz informacje o uzyciu
        return 1; // zakoncz z bledem
    } // koniec bloku brak argumentu

    ifstream in(argv[1]); // otworz plik wejsciowy
    if (!in) { // sprawdz czy plik sie otworzyl
        cerr << "Nie mozna otworzyc pliku.\n"; // komunikat o bledzie otwarcia
        return 1; // zakoncz z bledem
    } // koniec bloku bledu otwarcia

    vector<int> vals; // wektor wczytanych odleglosci
    int v; // zmienna na pojedyncza wartosc
    while (in >> v) vals.push_back(v); // wczytuj wartosci z pliku
    if (vals.empty()) { // sprawdz czy sa dane
        cerr << "Brak danych.\n"; // komunikat o braku danych
        return 1; // zakoncz z bledem
    } // koniec bloku braku danych

    for (int x : vals) { // sprawdz poprawnosc wartosci
        if (x <= 0) { // odleglosci musza byc dodatnie
            cerr << "Niepoprawna wartosc: " << x << "\n"; // komunikat o bledzie danych
            return 1; // zakoncz z bledem
        } // koniec bloku bledu wartosci
    } // koniec petli walidacji

    long long n = (long long)vals.size(); // liczba odleglosci
    long long disc = 1 + 8 * n; // wyznacznik do obliczenia liczby punktow
    long long s = sqrt((long double)disc); // pierwiastek z wyznacznika
    while (s * s < disc) s++; // korekta pierwiastka w gore
    while (s * s > disc) s--; // korekta pierwiastka w dol
    if (s * s != disc || (1 + s) % 2 != 0) { // sprawdz czy n pasuje do liczby punktow
        cerr << "Nieprawidlowa liczba elementow: " << n << "\n"; // komunikat o bledzie rozmiaru
        return 1; // zakoncz z bledem
    } // koniec bloku bledu rozmiaru

    int L = *max_element(vals.begin(), vals.end()); // maksymalna odleglosc to dlugosc
    Counts d, orig; // mapy zliczen odleglosci (robocza i oryginalna)
    for (int x : vals) { // przepisz odleglosci do map zliczen
        d[x]++; // zwieksz licznik w mapie roboczej
        orig[x]++; // zwieksz licznik w mapie oryginalnej
    } // koniec przepisywania do map
    if (d[L] != 1) { // dlugosc powinna wystapic dokladnie raz
        cerr << "Nieprawidlowa instancja: brak jednoznacznej dlugosci.\n"; // komunikat o bledzie instancji
        return 1; // zakoncz z bledem
    } // koniec sprawdzania dlugosci
    if (--d[L] == 0) d.erase(L); // usun wartosc L z mapy roboczej

    vector<int> points = {0, L}; // punkty skrajne rozwiazania
    Stats stats; // statystyki przeszukiwania
    auto start = chrono::steady_clock::now(); // start pomiaru czasu
    bool ok = solve(d, points, L, stats); // uruchom rozwiazywanie
    auto end = chrono::steady_clock::now(); // koniec pomiaru czasu
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count(); // czas w milisekundach
    cerr << "Czas: " << ms << " ms\n"; // wypisz czas na stderr
    if (!ok) { // brak rozwiazania
        cerr << "Nie znaleziono rozwiazania dla podanych elementow.\n"; // komunikat o braku rozwiazania
        cout << "Zgodnosc mapy: NIE\n"; // wypisz brak zgodnosci
        cout << "Liczba odwiedzonych wezlow: " << stats.nodes << "\n"; // wypisz liczbe wezlow
        cout << "Liczba sprawdzonych kandydatow: " << stats.candidates << "\n"; // wypisz liczbe kandydatow
        return 1; // zakoncz z bledem
    } // koniec bloku braku rozwiazania

    sort(points.begin(), points.end()); // posortuj punkty rosnaco
    vector<int> segs; // wektor odcinkow miedzy punktami
    for (size_t i = 1; i < points.size(); ++i) segs.push_back(points[i] - points[i - 1]); // zbuduj odcinki z punktow

    cout << "Dlugosc: " << L << "\n"; // wypisz dlugosc
    cout << "Mapa: "; // wypisz etykiete mapy
    for (size_t i = 0; i < segs.size(); ++i) { // petla po odcinkach
        if (i) cout << " "; // dodaj spacje miedzy odcinkami
        cout << segs[i]; // wypisz odcinek
    } // koniec petli po odcinkach
    cout << "\n"; // zakoncz linie mapy
    Counts built = buildCounts(points); // zbuduj mape odleglosci z rozwiazania
    cout << "Zgodnosc mapy: " << (built == orig ? "TAK" : "NIE") << "\n"; // porownaj z oryginalem
    cout << "Liczba odwiedzonych wezlow: " << stats.nodes << "\n"; // wypisz liczbe wezlow
    cout << "Liczba sprawdzonych kandydatow: " << stats.candidates << "\n"; // wypisz liczbe kandydatow
    return 0; // zakoncz poprawnie
} // koniec main
