# MSA + ACO (optapp)

Program konsolowy do rozwiazywania problemu Multiple Sequence Alignment (MSA)
z uzyciem metaheurystyki Ant Colony Optimization (ACO). Aplikacja dziala w trybie
menu i pozwala wczytywac dane z pliku, generowac instancje, uruchamiac ACO oraz
zapisywac dopasowanie do pliku.

## Kompilacja

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic optapp.cpp -o optapp
```

## Uruchomienie

```bash
./optapp
```

## Menu programu

1. Wczytaj sekwencje z pliku
2. Wygeneruj sekwencje
3. Pokaz aktualne sekwencje
4. Uruchom ACO
5. Zapisz ostatnie dopasowanie
0. Wyjscie

## Format pliku wejsciowego

Program obsluguje dwa formaty:

- FASTA: linie z nazwami rozpoczynaja sie od `>`, kolejne linie to sekwencja.
- Plik liniowy: kazda niepusta linia to jedna sekwencja.

W obu przypadkach znaki nie-alfabetyczne sa ignorowane, a sekwencja jest
czyszczona do wielkich liter.

## Generator sekwencji

Generator tworzy losowe sekwencje z podanego alfabetu (domyslnie ACGT).
Opcjonalnie wstawia wspolny motyw o zadanej dlugosci, z mozliwymi mutacjami
na poszczegolnych sekwencjach. Po wygenerowaniu mozna zapisac wynik do FASTA.

## Algorytm ACO

ACO konstruuje dopasowanie krok po kroku (kolumna po kolumnie). W kazdym kroku
wybierany jest ruch (ktore sekwencje przesuwamy w prawo) na podstawie:

- feromonu (pamiec o dobrych rozwiazaniach),
- heurystyki opartej o score kolumny (sum-of-pairs).

Po kazdej iteracji feromon paruje, a nastepnie jest wzmacniany przez najlepsza
mrowke z danej iteracji.

## Parametry ACO i scoring

Podczas uruchamiania ACO program pyta o:

- match/mismatch/gap: scoring kolumny (sum-of-pairs),
- liczbe mrowek i iteracji,
- parowanie (rho).

## Wyjscie

Program wypisuje najlepszy wynik, dlugosc dopasowania i czas obliczen.
Na zyczenie wyswietla dopasowanie oraz zapisuje je do pliku w formacie FASTA.

## Ograniczenia

- Liczba sekwencji < 63 (ograniczenie maski bitowej).
- Dla duzej liczby sekwencji ruchy sa ograniczane do sensownej liczby opcji,
  aby algorytm byl wydajny.
