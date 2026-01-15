## Instrukcja Użycia Programu `exe3`

Ten dokument wyjaśnia, jak skompilować i uruchomić program `exe3`, który implementuje heurystyczny algorytm do znajdowania lokalnego dopasowania wielu sekwencji nukleotydowych.

### Cel Programu

Program `exe3` ma za zadanie:
1.  Wczytać instancje składające się z sekwencji nukleotydowych (plik FASTA) i odpowiadających im ocen wiarygodności (plik QUAL).
2.  Usunąć z sekwencji nukleotydy o wiarygodności poniżej zadanego progu.
3.  Z przefiltrowanych sekwencji utworzyć graf, gdzie wierzchołki odpowiadają kilkuliterowym podciągom.
4.  Wyszukać w grafie, za pomocą algorytmu zachłannej budowy kliki, strukturę zbliżoną do kliki, w której każda sekwencja wejściowa jest reprezentowana dokładnie jednym wierzchołkiem.
5.  Wypisać znalezione fragmenty sekwencji wchodzące w skład tej struktury.

### Wymagania

*   Kompilator C++ (np. GCC/G++).
*   Pliki wejściowe w formatach FASTA (`.fasta`) i QUAL (`.qual`).

### Przygotowanie Plików Wejściowych (`.fasta` i `.qual`)

Program wymaga dwóch plików wejściowych dla każdej instancji:

1.  **Plik FASTA (`.fasta`):** Zawiera sekwencje nukleotydowe.
    *   Każda sekwencja zaczyna się od linii z identyfikatorem, poprzedzonej znakiem `>` (np. `>id_sekwencji_1`).
    *   Następne linie zawierają faktyczną sekwencję nukleotydów (A, C, G, T).
    *   **Przykład:**
        ```fasta
        >sekwencja_A
        AGCTAGCTAGCT
        >sekwencja_B
        GATCGATCGATC
        ```

2.  **Plik QUAL (`.qual`):** Zawiera oceny wiarygodności dla każdego nukleotydu w odpowiadającej sekwencji FASTA.
    *   Struktura pliku jest analogiczna do FASTA: każda sekwencja ocen zaczyna się od linii z identyfikatorem (takim samym jak w pliku FASTA), poprzedzonej znakiem `>`.
    *   Następne linie zawierają liczby całkowite, gdzie każda liczba odpowiada ocenie wiarygodności kolejnego nukleotydu w sekwencji FASTA.
    *   **Ważne:**
        *   Identyfikatory sekwencji w pliku `.qual` muszą być **takie same i w tej samej kolejności** jak w pliku `.fasta`.
        *   Liczba ocen wiarygodności dla danej sekwencji w pliku `.qual` musi być **równa długości** tej sekwencji w pliku `.fasta`.
    *   **Przykład (dla powyższego pliku FASTA):**
        ```qual
        >sekwencja_A
        20 22 18 25 30 15 28 29 21 24 26 19
        >sekwencja_B
        19 23 20 27 28 16 31 22 25 24 20 26
        ```

**Tworzenie instancji:** Zgodnie z `Description3.txt`, musisz samodzielnie przygotować co najmniej 5 instancji, modyfikując sekwencje tak, aby zawierały motywy (o długości kilkunastu nukleotydów) z niewielkimi różnicami między wystąpieniami. Pamiętaj o zachowaniu spójności długości sekwencji i ocen wiarygodności.

### Kompilacja Programu

Aby skompilować program, otwórz terminal w katalogu głównym projektu (`CombinatorialAlg`) i wykonaj następującą komendę:

```bash
g++ Exercise3/exe3.cpp -o Exercise3/exe3 -std=c++11
```

*   `g++`: Wywołuje kompilator G++.
*   `Exercise3/exe3.cpp`: Określa plik źródłowy do skompilowania.
*   `-o Exercise3/exe3`: Nadaje nazwę plikowi wykonywalnemu (`exe3`) i umieszcza go w katalogu `Exercise3`.
*   `-std=c++11`: Używa standardu C++11, zapewniając kompatybilność.

Po pomyślnej kompilacji, w katalogu `Exercise3` pojawi się plik wykonywalny `exe3`.

### Uruchamianie Programu

Program `exe3` przyjmuje cztery argumenty z linii komend. Składnia uruchomienia wygląda następująco:

```bash
./Exercise3/exe3 <plik_fasta> <plik_qual> <prog_wiarygodnosci> <dlugosc_podciagu>
```

**Wyjaśnienie argumentów:**

1.  `<plik_fasta>`: Ścieżka do pliku FASTA zawierającego sekwencje nukleotydowe.
2.  `<plik_qual>`: Ścieżka do pliku QUAL zawierającego oceny wiarygodności.
3.  `<prog_wiarygodnosci>`: **Liczba całkowita**. Nukleotydy, których ocena wiarygodności jest niższa niż ta wartość, zostaną usunięte z sekwencji przed analizą. Przykład: `20`.
4.  `<dlugosc_podciagu>`: **Liczba całkowita** (zalecany zakres od `4` do `9`). Określa długość podciągów, które będą tworzyć wierzchołki w grafie. Przykład: `7`.

**Przykład Użycia:**

Jeśli masz pliki `moje_sekwencje.fasta` i `moje_sekwencje.qual` w katalogu `Exercise3`, i chcesz uruchomić program z progiem wiarygodności `25` i długością podciągu `6`, komenda będzie wyglądać tak:

```bash
./Exercise3/exe3 Exercise3/moje_sekwencje.fasta Exercise3/moje_sekwencje.qual 25 6
```

### Wynik Działania Programu

Program wypisze na standardowe wyjście (konsolę) informacje o znalezionej strukturze kliki. Dla każdego wierzchołka należącego do znalezionej kliki, zostanie wyświetlona linia w formacie:

```
Sekwencja <numer_sekwencji> (<id_sekwencji>), pozycja <pozycja_poczatkowa>, fragment: <podciag_nukleotydowy>
```

*   `<numer_sekwencji>`: Numer sekwencji wejściowej (liczony od 1).
*   `<id_sekwencji>`: Identyfikator sekwencji pobrany z pliku FASTA.
*   `<pozycja_poczatkowa>`: Oryginalna pozycja początkowa podciągu w sekwencji wejściowej (przed filtrowaniem), liczona od 1.
*   `<podciag_nukleotydowy>`: Sam podciąg nukleotydowy, który został znaleziony jako część kliki.

---
Mam nadzieję, że ta instrukcja jest jasna i pomocna!