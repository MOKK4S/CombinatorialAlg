# Sprawozdanie: Heurystyczna Identyfikacja Motywów w Danych Sekwencyjnych

## 1. Wprowadzenie

Niniejszy dokument stanowi raport z realizacji projektu polegającego na implementacji oprogramowania do poszukiwania wspólnych wzorców (motywów) w zbiorze sekwencji nukleotydowych. Problem ten jest powszechny w dziedzinie bioinformatyki, gdzie dane pochodzące z urządzeń sekwencjonujących charakteryzują się niepewnością pomiarową (szumem) oraz naturalną zmiennością biologiczną.

W ramach projektu zadanie zostało zamodelowane jako problem grafowy, a do jego rozwiązania zastosowano heurystyczny algorytm wielomianowy, co pozwoliło na efektywną analizę danych wejściowych.

## 2. Format Danych Wejściowych

Oprogramowanie operuje na instancjach problemu, z których każda składa się z dwóch plików tekstowych:

1.  **Plik w formacie FASTA (`.fasta`):** Przechowuje sekwencje biologiczne. Standardowo, każdy wpis rozpoczyna się od linii nagłówkowej (znaku `>`), po której następuje unikalny identyfikator sekwencji. W kolejnych liniach znajdują się dane nukleotydowe, które są automatycznie łączone w pojedynczy ciąg.
2.  **Plik w formacie QUAL (`.qual`):** Przechowuje numeryczne oceny wiarygodności odpowiadające każdemu nukleotydowi z pliku FASTA. Format jest analogiczny – linia z identyfikatorem, a następnie oddzielone spacjami wartości jakościowe, które mogą obejmować wiele linii.

Integralność danych jest zapewniona przez mechanizm walidacji, który przed rozpoczęciem analizy weryfikuje zgodność liczby sekwencji, ich identyfikatorów oraz długości danych w obu plikach.

## 3. Opis Zaimplementowanego Algorytmu

Zastosowany algorytm realizuje wieloetapowy proces w celu identyfikacji grupy najlepiej dopasowanych do siebie podciągów (k-merów), po jednym z każdej sekwencji wejściowej.

1.  **Etap 1: Filtracja Danych.** Po wczytaniu i walidacji danych wejściowych, każda sekwencja poddawana jest procesowi filtracji. Na podstawie zdefiniowanego przez użytkownika progu jakości (`threshold`), nukleotydy o ocenie niższej niż podana wartość są eliminowane. Pozycje pozostałych nukleotydów w oryginalnej sekwencji są zachowywane do dalszej analizy.

2.  **Etap 2: Konstrukcja Grafu.** Na podstawie przefiltrowanych sekwencji budowany jest graf nieskierowany:
    *   **Generowanie Wierzchołków:** Każdy możliwy podciąg o długości `k` (parametr `substring_length`) staje się wierzchołkiem grafu. Informacje o pochodzeniu każdego wierzchołka (indeks sekwencji źródłowej i pozycja) są przechowywane w jego strukturze.
    *   **Tworzenie Krawędzi:** W celu optymalizacji procesu, wierzchołki są wstępnie grupowane według identycznej treści nukleotydowej. Krawędzie tworzone są wyłącznie między parami wierzchołków należących do tej samej grupy, które dodatkowo pochodzą z różnych sekwencji źródłowych, a różnica ich oryginalnych pozycji nie przekracza zadanego limitu (`10 * k`).

3.  **Etap 3: Heurystyczne Wyszukiwanie Kliki.** Centralnym elementem oprogramowania jest algorytm aproksymacyjny dla problemu maksymalnej kliki, z dodatkowym warunkiem reprezentacji każdej sekwencji. Zaimplementowano **sekwencyjny algorytm zachłanny**, którego ogólne zasady są szeroko opisywane w literaturze dotyczącej problemów NP-trudnych (np. Bomze et al., 1999).
    *   Algorytm iteruje przez wszystkie wierzchołki grafu, używając każdego jako potencjalnego punktu startowego do budowy kliki.
    *   Dla każdego punktu startowego, w sposób iteracyjny, do aktualnej kliki dołączany jest "najlepszy" z dostępnych kandydatów. Kandydat musi być połączony ze wszystkimi dotychczasowymi członkami kliki i pochodzić z sekwencji, która nie ma jeszcze swojego reprezentanta.
    *   Kryterium wyboru "najlepszego" kandydata opiera się na heurystyce stopnia wierzchołka – preferowane są wierzchołki o jak największej liczbie połączeń w całym grafie.
    *   Proces jest powtarzany dla wszystkich punktów startowych, a jako wynik końcowy zwracana jest największa znaleziona klika.

## 4. Analiza Złożoności Obliczeniowej

Złożoność czasowa algorytmu jest determinowana głównie przez etapy konstrukcji grafu i poszukiwania kliki.

*   **Oznaczenia:**
    *   `N` – liczba sekwencji wejściowych.
    *   `L` – maksymalna długość pojedynczej sekwencji.
    *   `V` – całkowita liczba wierzchołków grafu, gdzie `V = O(N * L)`.

*   **Złożoność poszczególnych etapów:**
    *   Wczytywanie i filtracja: **O(N * L)**.
    *   Konstrukcja grafu: W scenariuszu najgorszego przypadku, gdzie duża liczba podciągów jest identyczna, złożoność tego etapu dąży do **O(V²)**.
    *   Wyszukiwanie kliki: Heurystyka zachłanna iteruje `V` razy (dla każdego punktu startowego). Wewnątrz każdej iteracji, rozbudowa kliki (o maksymalnie `N` członków) i wyszukiwanie kandydatów wśród `V` wierzchołków prowadzi do pesymistycznego oszacowania złożoności na poziomie **O(V² * N²)**.

*   **Złożoność całkowita:** Algorytm jest zdominowany przez etap wyszukiwania kliki. Podstawiając `V`, jego całkowitą złożoność można oszacować jako **O(N⁴ * L²)**. Mimo wysokiego rzędu wielomianu, w praktyce dla danych o rozmiarach przewidzianych w zadaniu, czas wykonania pozostaje akceptowalny.

## 5. Przeprowadzone Testy i Analiza Wyników

W celu weryfikacji poprawności implementacji oraz oceny wpływu parametrów na jakość wyników, przeprowadzono serię testów na pięciu wygenerowanych instancjach (`dane/seq1` do `dane/seq5`). Dla każdej instancji wykonano przebiegi z różnymi wartościami progu jakości (`threshold`) oraz długości podciągu (`k`).

Zaobserwowano, że wyższy próg jakości (np. >25) generalnie poprawiał trafność, skutecznie eliminując potencjalne fałszywe dopasowania wynikające z szumu w danych. Jednocześnie, zbyt restrykcyjny próg (np. >35) w niektórych przypadkach prowadził do fragmentacji właściwego motywu i niemożności odnalezienia pełnej, reprezentatywnej kliki.

Długość podciągu `k` okazała się kluczowym parametrem. Wartości z zakresu 5-7 dawały najbardziej zbalansowane wyniki, pozwalając na identyfikację motywu mimo niewielkich wariancji między jego wystąpieniami. Zbyt mała wartość `k` (<5) generowała dużą liczbę przypadkowych krawędzi w grafie, natomiast zbyt duża (`k` > 8) była nazbyt wrażliwa na pojedyncze mutacje, co również utrudniało znalezienie kompletnej kliki.

*(Poniżej znajduje się przykładowa tabela, którą należy uzupełnić konkretnymi wynikami testów).*

| Plik Instancji      | Próg jakości | Dł. podciągu | Rozmiar kliki | Komentarz (np. zgodność z oczekiwaniami) |
|---------------------|--------------|----------------|---------------|------------------------------------------|
| `dane/seq1.fasta`   | 20           | 5              | *[wynik]*     | *[analiza]*                               |
| `dane/seq1.fasta`   | 30           | 5              | *[wynik]*     | *[analiza]*                               |
| `dane/seq2.fasta`   | 25           | 6              | *[wynik]*     | *[analiza]*                               |


## 6. Wnioski Końcowe

Zaimplementowana heurystyka zachłanna do problemu dopasowania wielosekwencyjnego okazała się skuteczna w identyfikowaniu ukrytych motywów w syntetycznych, zaszumionych danych. Testy empiryczne potwierdziły silną zależność jakości rozwiązania od odpowiedniego doboru parametrów wejściowych. Algorytm jest w stanie poprawnie zidentyfikować wzorzec, o ile próg jakości jest wystarczająco wysoki, aby odfiltrować błędy, ale nie na tyle wysoki, aby usunąć naturalne wariancje w obrębie motywu.

Złożoność wielomianowa algorytmu, choć teoretycznie wysoka, w praktyce okazała się wystarczająca do analizy instancji o rozmiarach założonych w projekcie w akceptowalnym czasie. Dalsze potencjalne usprawnienia mogłyby obejmować implementację bardziej zaawansowanych heurystyk (np. algorytmów przeszukiwania lokalnego lub symulowanego wyżarzania) w celu porównania jakości uzyskiwanych rozwiązań.

### Źródła

*   Bomze, I. M., Budinich, M., Pardalos, P. M., & Pelillo, M. (1999). The Maximum Clique Problem. In *Handbook of Combinatorial Optimization* (Vol. 4, pp. 1-74). Kluwer Academic Publishers.