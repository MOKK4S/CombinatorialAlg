# explainable.md

Ten plik opisuje, co robi kazda funkcja w kodach `exe4.cpp` i `exe4brut.cpp`,
od poczatku do konca. Oba programy rozwiazuja ten sam problem i maja te same
funkcje pomocnicze, roznia sie tylko tym, jak wybieraja kandydatow w `solve`.

## Typy i struktury

- `using Counts = map<int, int>;`
  - Mapa zliczen odleglosci. Klucz to odleglosc, wartosc to liczba wystapien.
- `struct Stats`
  - `nodes` liczy odwiedzone wezly drzewa przeszukiwania.
  - `candidates` liczy sprawdzone kandydaty punktow.

## Funkcje pomocnicze

- `restore(Counts& d, const vector<int>& removed)`
  - Przywraca do mapy `d` wszystkie odleglosci zapisane w `removed`.
  - To jest krok cofania w backtrackingu.

- `takeDistances(int p, const vector<int>& points, Counts& d, vector<int>& removed)`
  - Sprawdza, czy nowy punkt `p` da sie dodac do aktualnego rozwiazania.
  - Dla kazdego istniejacego punktu liczy odleglosc `abs(p - x)` i probuje
    odjac ja z mapy `d`.
  - Jesli jakiejkolwiek odleglosci brakuje, przywraca zmiany przez `restore`
    i zwraca `false`.
  - Gdy wszystko sie zgadza, zwraca `true` i zostawia usuniete odleglosci
    w `removed` (do ewentualnego cofniecia).

- `buildCounts(const vector<int>& points)`
  - Buduje multizbior odleglosci na podstawie odtworzonych punktow.
  - Liczy roznice `points[j] - points[i]` dla wszystkich par i zapisuje je w mapie.
  - Wynik sluzy do koncowej weryfikacji (zgodnosc mapy).

## Funkcja `solve` w `exe4.cpp` (wersja z ograniczonym rozgalezieniem)

- `solve(Counts& d, vector<int>& points, int L, Stats& stats)`
  - Zlicza kolejny wezel w `stats.nodes`.
  - Jezeli mapa `d` jest pusta, zwraca `true` (mamy kompletne rozwiazanie).
  - Wybiera najwieksza pozostala odleglosc `y` z `d`.
  - Probuje wstawic punkt w pozycji `y`:
    - `takeDistances` sprawdza zgodnosc z aktualnymi punktami.
    - Gdy sie uda, dodaje punkt, wywoluje rekurencje i w razie porazki
      cofa stan (`points.pop_back()` i `restore`).
  - Probuje wstawic punkt w pozycji `L - y` (druga mozliwosc), o ile jest rozna od `y`.
  - Jesli zadna proba nie da rozwiazania, zwraca `false`.

## Funkcja `solve` w `exe4brut.cpp` (wersja brute force)

- `solve(Counts& d, vector<int>& points, int L, Stats& stats)`
  - Zlicza kolejny wezel w `stats.nodes`.
  - Jezeli `d` jest pusta, zwraca `true`.
  - Buduje liste kandydatow z wszystkich kluczy mapy `d` i odwraca kolejnosc,
    aby sprawdzac od najwiekszych odleglosci.
  - Dla kazdego kandydata:
    - Pomija go, jesli punkt juz istnieje w `points`.
    - Sprawdza zgodnosc przez `takeDistances`.
    - Gdy sie uda, dodaje punkt, wchodzi w rekurencje i w razie porazki
      cofa stan (`points.pop_back()` i `restore`).
  - Jesli zaden kandydat nie prowadzi do rozwiazania, zwraca `false`.

## Funkcja `main` (obie wersje)

- Sprawdza argumenty, otwiera plik i wczytuje wszystkie liczby do `vals`.
- Waliduje dane:
  - wszystkie liczby musza byc dodatnie,
  - liczba elementow musi pasowac do wzoru na liczbe odleglosci.
- Wyznacza `L` jako maksymalna odleglosc.
- Buduje mapy zliczen `d` i `orig`, usuwa jedno wystapienie `L` z `d`.
- Inicjalizuje `points = {0, L}` i uruchamia `solve`.
- Mierzy czas, wypisuje statystyki i wynik.
- Na koncu sprawdza zgodnosc multizbioru odleglosci (`buildCounts` vs `orig`)
  i wypisuje `Zgodnosc mapy: TAK/NIE`.
