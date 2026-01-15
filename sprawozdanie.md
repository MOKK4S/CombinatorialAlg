# Sprawozdanie z Zadania 3: Heurystyczne Wyszukiwanie Motywów w Sekwencjach DNA

## 1. Opis Zastosowanego Algorytmu

Celem zaimplementowanego programu jest identyfikacja krótkiego, wspólnego wzorca (motywu) w zbiorze wielu sekwencji DNA. Program jest zaprojektowany do pracy z danymi, które mogą zawierać błędy i szumy (np. z sekwenatora), poprzez wstępną filtrację danych na podstawie ocen jakości nukleotydów. Do znalezienia motywu wykorzystano heurystyczny algorytm grafowy.

Proces działania programu można podzielić na następujące kroki:

1.  **Wczytanie i Walidacja Danych:** Program wczytuje dane z dwóch plików wejściowych: `.fasta` (sekwencje nukleotydowe) i `.qual` (oceny wiarygodności). Następnie przeprowadza walidację, sprawdzając, czy liczba sekwencji, ich identyfikatory oraz długości są spójne w obu plikach.

2.  **Filtracja Sekwencji:** Każda sekwencja jest filtrowana na podstawie podanego przez użytkownika progu wiarygodności (`threshold`). Nukleotydy, których ocena jakości jest poniżej progu, są usuwane. Program zapamiętuje oryginalne pozycje (indeksy + 1) każdego nukleotydu, który pozostał po filtracji.

3.  **Generowanie Wierzchołków Grafu:** Z przefiltrowanych sekwencji generowane są wszystkie możliwe podciągi (k-mery) o zadanej przez użytkownika długości (`substring_length`). Każde wystąpienie takiego podciągu staje się osobnym wierzchołkiem w grafie. Dla każdego wierzchołka przechowywana jest informacja o jego pochodzeniu (numer sekwencji i oryginalna pozycja).

4.  **Konstrukcja Grafu:** Krawędzie w grafie tworzone są pomiędzy wierzchołkami, które spełniają łącznie trzy warunki:
    *   Reprezentują identyczny podciąg nukleotydowy.
    *   Pochodzą z **różnych** sekwencji wejściowych.
    *   Różnica między ich oryginalnymi pozycjami w sekwencjach jest nie większa niż dziesięciokrotność długości podciągu.

5.  **Wyszukiwanie Kliki (Heurystyka Zachłannej Budowy):** Jest to kluczowy element algorytmu. Celem jest znalezienie w grafie kliki (lub struktury do niej zbliżonej), w której każda sekwencja wejściowa jest reprezentowana przez dokładnie jeden wierzchołek. Zastosowana heurystyka to **algorytm zachłannej budowy kliki**:
    *   Algorytm iteruje przez każdy wierzchołek grafu, traktując go jako potencjalny "zalążek" nowej kliki. Pozwala to uniknąć uzależnienia wyniku od jednego, być może nieoptymalnego, punktu startowego.
    *   Dla każdego wierzchołka startowego algorytm iteracyjnie próbuje rozszerzyć klikę. W każdym kroku tworzona jest lista "kandydatów" na nowego członka kliki.
    *   Kandydat musi spełniać dwa warunki: pochodzić z sekwencji, która nie jest jeszcze reprezentowana w budowanej klice, oraz być połączony krawędzią ze **wszystkimi** dotychczasowymi członkami kliki.
    *   Jeśli lista kandydatów nie jest pusta, wybierany jest z niej "najlepszy" kandydat – w tej implementacji jest to wierzchołek o najwyższym stopniu (największej liczbie połączeń w całym grafie). Taki wybór faworyzuje wierzchołki, które są centralnymi punktami w swoich regionach grafu.
    *   Proces dołączania jest powtarzany do momentu, aż nie będzie można znaleźć więcej pasujących kandydatów.
    *   Na koniec algorytm porównuje wielkość właśnie zbudowanej kliki z największą znalezioną do tej pory i w razie potrzeby aktualizuje najlepszy wynik.

Wynikiem działania programu są informacje o wierzchołkach należących do największej znalezionej w ten sposób kliki.

## 2. Oszacowanie Złożoności Obliczeniowej

Przyjmijmy następujące oznaczenia:
*   `N` – liczba sekwencji wejściowych.
*   `L` – maksymalna długość pojedynczej sekwencji.
*   `k` – długość podciągu (parametru `substring_length`).

Złożoność poszczególnych etapów algorytmu:

1.  **Wczytywanie i Filtracja:** Operacje te wymagają jednokrotnego przetworzenia każdej sekwencji. Złożoność wynosi **O(N * L)**.

2.  **Generowanie Wierzchołków:** Z każdej przefiltrowanej sekwencji generowane jest w przybliżeniu `L - k + 1` wierzchołków. Całkowita liczba wierzchołków `V` jest więc rzędu **O(N * L)**.

3.  **Konstrukcja Grafu:** W zaimplementowanym podejściu najpierw grupujemy wierzchołki według ich treści. Następnie dla każdej grupy porównujemy wszystkie pary wierzchołków. W najgorszym przypadku, gdy wszystkie podciągi są identyczne, wszystkie `V` wierzchołków trafia do jednej grupy, co prowadzi do złożoności **O(V²)**, czyli **O((N * L)²)**.

4.  **Wyszukiwanie Kliki:** Jest to najbardziej kosztowny obliczeniowo etap.
    *   Zewnętrzna pętla algorytmu iteruje przez każdy wierzchołek, aby potraktować go jako punkt startowy, co daje `V` iteracji.
    *   Wewnętrzna pętla `while` wykonuje się co najwyżej `N` razy (bo klika nie może mieć więcej wierzchołków niż liczba sekwencji).
    *   Wewnątrz tej pętli, aby znaleźć kandydatów, musimy przejrzeć wszystkie `V` wierzchołków i dla każdego z nich sprawdzić połączenia z aktualnymi członkami kliki (których jest co najwyżej `N`).
    *   Daje to przybliżoną złożoność rzędu `V * N * V * N`, czyli **O(V² * N²)**.

**Całkowita złożoność algorytmu** jest zdominowana przez etap wyszukiwania kliki i wynosi w przybliżeniu **O(V² * N²) = O((N * L)² * N²) = O(N⁴ * L²)**. Jest to złożoność wielomianowa, zgodna z założeniami zadania.

## 3. Przykładowe Testy

*Ta sekcja powinna zostać wypełniona na podstawie faktycznie przeprowadzonych testów.*

### Instancja 1

*   **Dane wejściowe:**
    *   **(Tutaj wklej zawartość pliku .fasta dla instancji 1, z zaznaczonym motywem)**
    *   **(Tutaj wklej zawartość pliku .qual dla instancji 1, z zaznaczonymi ocenami dla motywu)**

*   **Test 1.1:**
    *   Parametry: `threshold = [WARTOŚĆ]`, `substring_length = [WARTOŚĆ]`
    *   Wynik: `(Wklej tutaj wynik działania programu)`
    *   Analiza: `(Opisz, czy wynik jest zgodny z oczekiwaniami)`

*   **Test 1.2:**
    *   Parametry: `threshold = [WARTOŚĆ]`, `substring_length = [WARTOŚĆ]`
    *   Wynik: `(Wklej tutaj wynik działania programu)`
    *   Analiza: `(Opisz, czy wynik jest zgodny z oczekiwaniami)`

*   **Test 1.3:**
    *   Parametry: `threshold = [WARTOŚĆ]`, `substring_length = [WARTOŚĆ]`
    *   Wynik: `(Wklej tutaj wynik działania programu)`
    *   Analiza: `(Opisz, czy wynik jest zgodny z oczekiwaniami)`

---

### Instancja 2

*(Struktura jak dla Instancji 1)*

---

### Instancja 3

*(Struktura jak dla Instancji 1)*

## 4. Wnioski

*(Ta sekcja powinna zostać wypełniona na podstawie obserwacji z przeprowadzonych testów).*

Należy tutaj opisać, jak zmiana parametrów wejściowych (`prog_wiarygodnosci` i `dlugosc_podciagu`) wpływa na uzyskane wyniki. Przykładowe punkty do analizy:

*   Jak wysoki próg wiarygodności wpływa na zdolność programu do znalezienia motywu? Czy usunięcie zbyt wielu nukleotydów "psuje" motyw?
*   Jak niski próg wiarygodności (pozostawienie "szumu") wpływa na wynik? Czy program znajduje fałszywe dopasowania?
*   Jak długość podciągu (`substring_length`) wpływa na wynik? Dla jakich długości program radzi sobie najlepiej ze znalezieniem wprowadzonego motywu, który zawiera drobne różnice?
*   Czy zaobserwowano sytuacje, w których algorytm nie znalazł żadnej kliki lub znalazł klikę o rozmiarze mniejszym niż liczba sekwencji? Jakie mogły być tego przyczyny?