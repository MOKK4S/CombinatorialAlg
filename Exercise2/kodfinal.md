# razin (komentarze i pseudokod)

Program wczytuje skierowany graf z pliku `graphX.txt` (lub `graph.txt`), sprawdza, czy jest to graf sprzężony (G‑graf) oraz czy jest liniowy. Wynik zapisuje do `graph_OutputX.txt` w tym samym dwukolumnowym formacie `N M` + lista łuków.

## Format plików
- Pierwsza linia: `N M` (liczba wierzchołków, liczba łuków).
- Kolejne `M` linii: `u v` (łuk z `u` do `v`).
- Dopuszczalne indeksowanie 0- lub 1‑bazowe; wewnętrznie normalizujemy do 0‑bazowego.

## Pseudokod funkcji

### load_graph(path)
1. Otwórz plik; jeśli się nie uda, zwróć `false`.
2. Wczytaj `N` i `M`; jeśli błąd, zwróć `false`.
3. Czytaj `M` łuków do listy; zapamiętaj, czy pojawiło się `0` (oznacza indeksowanie od zera).
4. Ustal `offset = 0` jeśli pojawiło się `0`, w przeciwnym razie `offset = 1`.
5. Utwórz `graph` jako wektor `N` pustych list.
6. Dla każdego łuku `(u, v)` odejmij `offset`; jeśli któryś koniec wychodzi poza zakres `0..N-1`, zgłoś komunikat i pomiń; w przeciwnym razie dodaj `v` do listy sąsiedztwa `u`.
7. Zwróć `true`.

### save_graph(graph, path)
1. Otwórz plik do zapisu; w razie błędu zwróć `false`.
2. Policz `N = graph.size()` i `M = suma długości list sąsiedztwa`.
3. Zapisz linię `N M`.
4. Dla każdej krawędzi `(u, v)` w 0‑bazowym grafie zapisz `u+1 v+1` (1‑indeksowo).
5. Zwróć `true`.

### is_G_graph(graph)
1. Dla każdej pary różnych wierzchołków `i < j`:
   - Posortuj ich listy następników.
   - Wyznacz część wspólną następników.
   - Jeśli część wspólna niepusta **i** listy następników nie są identyczne, zwróć `false`.
2. Jeśli nie było konfliktu, zwróć `true`.

### build_predecessors(graph)
1. Utwórz wektor `predecessors` o rozmiarze `N`, każdy element pusty.
2. Dla każdej krawędzi `u -> v` dodaj `u` do `predecessors[v]`.
3. Zwróć `predecessors`.

### is_line_graph(graph)
1. Zbuduj `predecessors = build_predecessors(graph)`.
2. Dla każdej pary wierzchołków `i < j`:
   - Posortuj listy następców `i` i `j`.
   - Posortuj listy poprzedników `i` i `j`.
   - Wyznacz część wspólną następców oraz część wspólną poprzedników.
   - Jeśli część wspólna następców jest niepusta **i** (listy następców różne **lub** część wspólna poprzedników niepusta), zwróć `false`.
3. W przeciwnym razie zwróć `true`.

### build_H_edges(graph)
1. Dla każdego wierzchołka `i` utwórz łuk `(2*i, 2*i+1)` i zapisz w liście `edges`.
2. Dla każdej krawędzi `i -> v` w grafie:
   - `source = edges[i].second` (głowa krawędzi `i`),
   - `target = edges[v].first` (ogon krawędzi `v`),
   - w całej liście `edges` zamień każdy koniec równy `target` na `source` (sklejanie).
3. Zwróć zmodyfikowaną listę `edges`.

### build_adjacency_list(edges)
1. Zbierz wszystkie końce łuków, usuń duplikaty, posortuj.
2. Zbuduj mapowanie `stary -> nowy` na zakres 0..K-1.
3. Utwórz `adjacency` o rozmiarze `K` pustych list.
4. Dla każdego łuku `(u, v)` dopisz `mapped(u) -> mapped(v)` do `adjacency`.
5. Zwróć `adjacency`.

### main()
1. Zapytaj użytkownika o numer grafu; skonstruuj `input_name` (`graphX.txt` lub `graph.txt`) i `output_name` (`graph_OutputX.txt`).
2. Wczytaj graf `G` z `input_name` funkcją `load_graph`; w razie błędu zakończ.
3. Zapisz kopię wejścia do `output_name` (dwukolumnowo).
4. Jeśli `is_G_graph(G)` jest prawdą:
   - Wyświetl, że graf jest sprzężony.
   - Sprawdź `is_line_graph(G)` i wypisz wynik.
   - Zbuduj `H = build_adjacency_list(build_H_edges(G))` i zapisz do `output_name` (nadpisanie).
5. W przeciwnym razie wypisz, że graf nie jest sprzężony.
