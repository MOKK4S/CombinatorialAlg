   # Sprawozdanie – Exercise 2 (grafy sprzężone)

   ## Format danych
   Grafy zapisujemy w prostym formacie tekstowym. Pierwsza linia zawiera `N M`, czyli liczbę wierzchołków i łuków. W kolejnych wierszach występują pary `u v` opisujące łuki (numeracja 1‑indeksowa). Dopuszczamy pętle i wielokrotne łuki – w zadaniu trzeba było obsłużyć wszystkie takie przypadki. Po każdej ręcznej edycji plik można szybko zweryfikować poleceniem `./build/exercise2 graf.txt`; jeśli nagłówek nie pasuje do zawartości, program zgłasza to natychmiast.

   ## Opis algorytmu

   1. **Wczytanie i normalizacja.** Program odczytuje `N` i `M`, wczytuje wszystkie łuki i normalizuje numerację (jeśli plik był 1‑indeksowy, odejmujemy jedynkę). W tym kroku powstają listy następców i poprzedników dla każdego wierzchołka.
   2. **Odtwarzanie grafu H.** Każdy wierzchołek grafu sprzężonego G traktowany jest jako łuk w grafie H. Tworzymy więc dwa końce (początek i koniec łuku) i łączymy je zgodnie z relacją „koniec jednego łuku przechodzi w początek kolejnego”. Sklejanie realizujemy strukturą DSU; po przejściu całej listy łuków powstaje pełna lista łuków w grafie H.
   3. **Weryfikacja + test liniowości.** Budujemy graf liniowy `L(H)` i porównujemy go z wejściowym G. Jeśli multizbiory łuków są identyczne, G jest grafem sprzężonym. Następnie liczymy stopnie wierzchołków H (pętle wnoszą dwa). Gdy każdy wierzchołek ma stopień co najwyżej 2, uznajemy graf za liniowy.

   ## Złożoność
   Wczytanie danych i normalizacja to `O(N + M)`. Rekonstrukcja H za pomocą DSU ma koszt `O(M α(N))`. Budowa `L(H)` wymaga w najgorszym razie `O(M²)` porównań, ale przy limitach zadania (≤20 wierzchołków, ≤30 łuków) jest to pomijalne. Porównanie grafów to `O(M log M)`, a test liniowości `O(M)`. Całość działa w czasie wielomianowym i w praktyce kończy się błyskawicznie.

   ## Testy
   Przygotowaliśmy dziesięć grafów sprzężonych (pliki `graph1.txt`–`graph5.txt` oraz `graph11.txt`–`graph15.txt`) oraz pięć grafów niesprzężonych (`graph6.txt`–`graph10.txt`). Każdy ma co najmniej 10 wierzchołków. W grafach dodatnich pojawiają się pętle, wielokrotne łuki, długie cykle i przeplatające się ścieżki; dodatkowo pięć najnowszych (`graph11`–`graph15`) odwzorowuje grafy oryginalne H, które są liniowe (każdy wierzchołek ma stopień co najwyżej 2). Grafy ujemne zostały wylosowane – brak w nich struktury, którą dałoby się odwzorować jako graf liniowy.

   | Plik        | \|V\| | \|E\| | Sprzężony | Liniowy | Komentarz |
   |-------------|-------|-------|-----------|---------|-----------|
   | graph1.txt  | 10    | 20    | tak       | nie     | cykle z przekątnymi |
   | graph2.txt  | 12    | 24    | tak       | nie     | gęsta sieć cykli |
   | graph3.txt  | 12    | 25    | tak       | nie     | pętle i odgałęzienia |
   | graph4.txt  | 12    | 29    | tak       | nie     | liczne łuki równoległe |
   | graph5.txt  | 12    | 27    | tak       | nie     | multigraf z duplikatami |
   | graph6.txt  | 12    | 16    | nie       | –       | losowy graf |
   | graph7.txt  | 11    | 15    | nie       | –       | rzadsza struktura |
   | graph8.txt  | 12    | 18    | nie       | –       | losowy graf z pętlami |
   | graph9.txt  | 10    | 15    | nie       | –       | mieszane stopnie |
   | graph10.txt | 10    | 25    | nie       | –       | bardzo gęsty, ale niesprzężony |
   | graph11.txt | 11    | 10    | tak       | tak     | liniowy graf oryginalny (łańcuch) |
   | graph12.txt | 12    | 22    | tak       | tak     | liniowy H z cyklem i powrotami |
   | graph13.txt | 10    | 14    | tak       | tak     | liniowy H z pętlami i odbiciami |
   | graph14.txt | 11    | 15    | tak       | tak     | liniowy H z symetrycznymi przejściami |
   | graph15.txt | 12    | 22    | tak       | tak     | liniowy H z wieloma powrotami |

   Każdy plik pozytywny został dodatkowo sprawdzony ręcznie: `graph_outN.txt` pokrywa się z grafem H, z którego powstał `graphN.txt`. Dla grafów niesprzężonych program wypisuje „Graph is conjugated: false” i niczego nie zapisuje.

   ## Wizualizacje
   W finalnej wersji raportu planujemy zamieścić co najmniej cztery pary ilustracji. Każda para to graf wejściowy G i odpowiadający mu graf H, a łuki H podpisujemy etykietami `v1`, `v2` itd. Unikamy „podwójnych strzałek”: przeciwne kierunki rysujemy osobno.

   ### Przykład graph1.txt
   Struktura G składa się z dziesięciu wierzchołków reprezentujących łuki w H. Widoczne są długie cykle oraz przekątne, które wymuszają konkretne sklejenia.

   `[zdjecie{Graf wejściowy G dla graph1.txt}]`

   Odzyskany graf H pokazuje kilka wierzchołków o większym stopniu, a każdy łuk otrzymuje etykietę `v1..v10`.

   `[zdjecie{Graf H dla graph1.txt}]`

   ### Przykład graph2.txt
   Drugie wejście to gęsty graf liniowy (12 wierzchołków, 24 łuki). Na wizualizacji G łatwo zauważyć krótkie cykle i połączenia zwrotne.

   `[zdjecie{Graf G dla graph2.txt}]`

   W grafie H widać, że żaden wierzchołek nie spełnia warunku liniowości, co program raportuje jako „false”.

   `[zdjecie{Graf H dla graph2.txt}]`

   ### Przykład graph3.txt
   Trzeci przykład zawiera pętle własne – rysunek G pokazuje, które wierzchołki łączą się same ze sobą.

   `[zdjecie{Graf G dla graph3.txt}]`

   Rysunek H ujawnia pochodzenie tych pętli. Etkiety łuków `v1..v12` pozwalają łatwo powiązać wierzchołki G z łukami H.

   `[zdjecie{Graf H dla graph3.txt}]`

   ### Przykład graph4.txt
   Ostatni z pokazanych grafów sprzężonych zawiera wiele równoległych łuków. W G wygląda to na skomplikowaną sieć, ale H ma logiczną strukturę.

   `[zdjecie{Graf G dla graph4.txt}]`

   `[zdjecie{Graf H dla graph4.txt}]`

   ## Wnioski
   Program spełnia wszystkie wymagania zadania. Dla grafów sprzężonych poprawnie odzyskuje graf H i sprawdza liniowość. Dla grafów niesprzężonych kończy działanie po wczytaniu danych, dzięki czemu nie nadpisujemy wcześniejszych wyników. Jedynym teoretycznym ograniczeniem jest kwadratowy koszt budowy grafu liniowego, który przy aktualnych rozmiarach danych jest niezauważalny. Dalej planujemy przygotować automatyczne wizualizacje (np. Graphviz) i zestaw testów uruchamianych przed każdą zmianą w kodzie, żeby utrzymać spójność dokumentacji.***
