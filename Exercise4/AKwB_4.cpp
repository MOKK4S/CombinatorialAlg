#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>
#include <iomanip>

using namespace std;
// Struktura realizujaca rekonstrukcje mapy restrykcyjnej
struct Mapowanie
{
    vector<int> odleglosci;         // wielokrotny zbior odleglosci (wektor wejsciowy)
    vector<bool> czy_uzyte;         // flaga czy dana pozycja w 'odleglosci' jest juz uzyta
    vector<int> mapa_ciec;          // aktualnie budowana mapa (pozycje ciec od lewej krawedzi)
    vector<int> mapa_odcinkow;      // odcinki pomiedzy kolejnymi punktami (mapa jako wektor odcinkow)
    int liczba_ciec;                // liczba elementow mapy (ile miejsc ma mapa)
    int dlugosc_fragmentu;          // dlugosc fragmentu (najwiekszy element w odleglosci)
    // Liczniki diagnostyczne
    long long licznik_wezlow;       // liczba odwiedzonych wezlow w drzewie przeszukiwania
    long long licznik_sprawdzen;    // liczba prob dopasowan / kandydatow sprawdzonych

    // Konstruktor: przyjmuje wektor odleglosci (moze zawierac duplikaty)
    Mapowanie(const vector<int>& wejscie)
    {
        odleglosci = wejscie;
        sort(odleglosci.begin(), odleglosci.end()); // sortujemy rosnaco, ulatwi to prace
        czy_uzyte.assign(odleglosci.size(), false);
        liczba_ciec = 0;
        dlugosc_fragmentu = 0;
        licznik_wezlow = 0;
        licznik_sprawdzen = 0;
    }

    // Znajdz indeks pierwszego nieuzytego elementu o wartosci v; zwraca -1 jesli brak
    int znajdz_nieuzyty(int v)
    {
        for (size_t i = 0; i < odleglosci.size(); ++i)
            if (!czy_uzyte[i] && odleglosci[i] == v)
                return (int)i;
        return -1;
    }

    // Znajdz indeks ostatniego elementu o wartosci v (uzytego lub nie)
    int znajdz_ostatni(int v)
    {
        for (int i = (int)odleglosci.size() - 1; i >= 0; --i)
            if (odleglosci[i] == v)
                return i;
        return -1;
    }

    // Sprawdza czy mozna dopisac punkt pozycji; jesli tak oznacza wymagane odleglosci jako uzyte
    bool sprawdz_i_oznacz(int poz, vector<int>& zaznaczone)
    {
        vector<int> punkty;
        punkty.push_back(0); // lewy koniec
        for (int i = 0; i < (int)mapa_ciec.size(); ++i)
            if (mapa_ciec[i] != -1)
                punkty.push_back(mapa_ciec[i]);
        punkty.push_back(dlugosc_fragmentu); // prawy koniec

        for (int q : punkty)
        {
            int d = abs(poz - q); // poszukiwana odleglosc
            int idx = znajdz_nieuzyty(d);
            if (idx == -1)
            {
                for (int j : zaznaczone) czy_uzyte[j] = false;
                zaznaczone.clear();
                return false;
            }
            czy_uzyte[idx] = true;
            zaznaczone.push_back(idx);
        }
        return true;
    }

    // Procedura rekurencyjna (dokladnie wg wzoru: void szukaj(int ind,int *jest))
    void szukaj(int ind, int* czy_znaleziono)
    {
        // Zwiekszamy licznik odwiedzin: kazde wywolanie funkcji to jeden wezel
        ++licznik_wezlow;
        if (ind == liczba_ciec)
        {
            // Zbuduj posortowana liste punktow (0, pozycje ciec, dlugosc_fragmentu)
            vector<int> punkty;
            punkty.push_back(0);
            for (int i = 0; i < liczba_ciec; ++i)
                if (mapa_ciec[i] != -1) punkty.push_back(mapa_ciec[i]);
            punkty.push_back(dlugosc_fragmentu);
            sort(punkty.begin(), punkty.end());

            cout << "Znalezione punkty: ";
            for (size_t i = 0; i < punkty.size(); ++i)
            {
                if (i) cout << " ";
                cout << punkty[i];
            }
            cout << "\n";

            // Oblicz odcinki (roznice miedzy kolejnymi punktami) i zapisz jako 'mapa'
            mapa_odcinkow.clear();
            for (size_t k = 1; k < punkty.size(); ++k)
                mapa_odcinkow.push_back(punkty[k] - punkty[k - 1]);

            cout << "Znaleziono mape (odcinki): ";
            for (size_t k = 0; k < mapa_odcinkow.size(); ++k)
            {
                if (k) cout << " ";
                cout << mapa_odcinkow[k];
            }
            cout << "\n";

            // Zachowaj takze (i wypisz) oryginalne pozycje ciec w osobnej zmiennej
            cout << "Pozycje ciec: ";
            for (int i = 0; i < liczba_ciec; ++i)
            {
                if (i) cout << " ";
                cout << mapa_ciec[i];
            }
            cout << "\n";

            // Weryfikacja: oblicz wszystkie odleglosci miedzy punktami i porownaj z wejsciowym multizbiorem
            vector<int> dystanse;
            for (size_t a = 0; a < punkty.size(); ++a)
                for (size_t b = a + 1; b < punkty.size(); ++b)
                    dystanse.push_back(abs(punkty[b] - punkty[a]));
            sort(dystanse.begin(), dystanse.end());

            bool zgodne = (dystanse == odleglosci);
            cout << "Weryfikacja zgodnosci mapy: " << (zgodne ? "TAK" : "NIE") << "\n";
            if (!zgodne)
            {
                cout << "Oczekiwany multizbior: ";
                for (size_t i = 0; i < odleglosci.size(); ++i) { if (i) cout << " "; cout << odleglosci[i]; }
                cout << "\n";
                cout << "Obliczone dystanse: ";
                for (size_t i = 0; i < dystanse.size(); ++i) { if (i) cout << " "; cout << dystanse[i]; }
                cout << "\n";
            }

            *czy_znaleziono = 1;
        }
        else
        {
            for (size_t i = 0; i < odleglosci.size(); ++i)
            {
                if (czy_uzyte[i]) continue;
                int kandydat = odleglosci[i];
                // Zliczamy kazda probe wstawienia kandydata
                ++licznik_sprawdzen;
                if (kandydat <= 0 || kandydat >= dlugosc_fragmentu) continue;

                bool juzJest = false;
                for (int j = 0; j < liczba_ciec; ++j)
                    if (mapa_ciec[j] == kandydat) { juzJest = true; break; }
                if (juzJest) continue;

                vector<int> zaznaczone;
                if (sprawdz_i_oznacz(kandydat, zaznaczone))
                {
                    mapa_ciec[ind] = kandydat;
                    szukaj(ind + 1, czy_znaleziono);
                    if (*czy_znaleziono == 1) break;
                    mapa_ciec[ind] = -1;
                    for (int idx : zaznaczone) czy_uzyte[idx] = false;
                }
            }
        }
    }
};

// Oblicza m takie, ze C(m,2) == liczba_elementow_wejsciowych
int oblicz_liczbe_punktow(int liczba_elementow_wejsciowych)
{
    for (int m = 2; m < 2000; ++m)
        if ((m * (m - 1)) / 2 == liczba_elementow_wejsciowych) return m;
    return -1;
}



int main(int argc, char** argv)
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string nazwaPliku;
    if (argc > 1) nazwaPliku = argv[1];
    else
    {
    cout << "Podaj nazwe pliku z instancja: ";
    cin >> nazwaPliku;
    }

    ifstream fin(nazwaPliku);
    if (!fin)
    {
        cerr << "Nie mozna otworzyc pliku: " << nazwaPliku << "\n";
        return 1;
    }
    vector<int> dane;
    int v;
    while (fin >> v) dane.push_back(v);
    fin.close();

    if (dane.empty()) { cerr << "Plik nie zawiera liczb." << "\n"; return 1; }

    // multizbior wejsciowy (dokladnie tak jak wczytano)
    cout << "Multizbior wejsciowy: ";
    for (size_t i = 0; i < dane.size(); ++i)
    {
        if (i) cout << " ";
        cout << dane[i];
    }
    cout << "\n";


    int liczba_elementow_wejsciowych = (int)dane.size();
    int liczba_punktow = oblicz_liczbe_punktow(liczba_elementow_wejsciowych);
    if (liczba_punktow == -1) { cerr << "Nieprawidlowa liczba elementow wejsciowych." << "\n"; return 1; }

    int liczba_ciec = liczba_punktow - 2;
    cout << "liczba punktow = " << liczba_punktow << ", liczba ciec = " << liczba_ciec << "\n";


    Mapowanie odtwarzacz_mapy(dane);
    odtwarzacz_mapy.liczba_ciec = liczba_ciec;
    odtwarzacz_mapy.mapa_ciec.assign(liczba_ciec, -1);
    odtwarzacz_mapy.dlugosc_fragmentu = odtwarzacz_mapy.odleglosci.empty() ? 0 : odtwarzacz_mapy.odleglosci.back();

    int indeks_dlugosci = odtwarzacz_mapy.znajdz_ostatni(odtwarzacz_mapy.dlugosc_fragmentu);
    if (indeks_dlugosci == -1) { cerr << "Brak maksymalnego elementu (dlugosci) w zbiorze." << "\n"; return 1; }
    odtwarzacz_mapy.czy_uzyte[indeks_dlugosci] = true;

    int znaleziono = 0;
    // Timer bedzie uruchamiany tuz przed pierwszym wywolaniem funkcji szukaj
    bool timer_uruchomiony = false;
    chrono::high_resolution_clock::time_point czas_start;
    chrono::high_resolution_clock::time_point czas_koniec;

    if ((int)odtwarzacz_mapy.odleglosci.size() >= 2)
    {
        int drugi_najwiekszy = odtwarzacz_mapy.odleglosci[(int)odtwarzacz_mapy.odleglosci.size() - 2];
        int pierwszy_punkt = odtwarzacz_mapy.dlugosc_fragmentu - drugi_najwiekszy;

        int indeks_pierwszego = odtwarzacz_mapy.znajdz_nieuzyty(pierwszy_punkt);
        int indeks_drugiego = odtwarzacz_mapy.znajdz_nieuzyty(odtwarzacz_mapy.dlugosc_fragmentu - pierwszy_punkt);

        if (indeks_pierwszego != -1 && indeks_drugiego != -1)
        {
            vector<int> zaznaczone_indeksy_local;
            odtwarzacz_mapy.czy_uzyte[indeks_pierwszego] = true;
            zaznaczone_indeksy_local.push_back(indeks_pierwszego);
            odtwarzacz_mapy.czy_uzyte[indeks_drugiego] = true;
            zaznaczone_indeksy_local.push_back(indeks_drugiego);

            odtwarzacz_mapy.mapa_ciec[0] = pierwszy_punkt;
            if (!timer_uruchomiony) { czas_start = chrono::high_resolution_clock::now(); timer_uruchomiony = true; }
            odtwarzacz_mapy.szukaj(1, &znaleziono);

            if (znaleziono == 0)
            {
                for (int id : zaznaczone_indeksy_local) odtwarzacz_mapy.czy_uzyte[id] = false;
                odtwarzacz_mapy.mapa_ciec[0] = -1;
            }
        }
    }

    if (znaleziono == 0)
    {
        for (size_t i = 0; i < odtwarzacz_mapy.odleglosci.size(); ++i) odtwarzacz_mapy.czy_uzyte[i] = false;
        odtwarzacz_mapy.czy_uzyte[indeks_dlugosci] = true;
        odtwarzacz_mapy.mapa_ciec.assign(liczba_ciec, -1);

        if (!timer_uruchomiony) { czas_start = chrono::high_resolution_clock::now(); timer_uruchomiony = true; }
        odtwarzacz_mapy.szukaj(0, &znaleziono);
    }

    if (znaleziono == 0) cout << "Brak rozwiazania (dane moga byc bledne)." << "\n";

    if (timer_uruchomiony)
    {
        czas_koniec = chrono::high_resolution_clock::now();
        chrono::duration<double> czas = czas_koniec - czas_start;
        cout << "Liczba odwiedzonych wezlow: " << odtwarzacz_mapy.licznik_wezlow << "\n";
        cout << "Liczba sprawdzonych kandydatow: " << odtwarzacz_mapy.licznik_sprawdzen << "\n";
        cout << fixed << setprecision(3) << "Czas dzialania: " << czas.count() << " s\n";
    }

    return 0;
}

