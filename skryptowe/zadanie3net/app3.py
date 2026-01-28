import math
import os

CODON_PAIRS = (
    ("ATA", "I"), ("ATC", "I"), ("ATT", "I"), ("ATG", "M"),
    ("ACA", "T"), ("ACC", "T"), ("ACG", "T"), ("ACT", "T"),
    ("AAC", "N"), ("AAT", "N"), ("AAA", "K"), ("AAG", "K"),
    ("AGC", "S"), ("AGT", "S"), ("AGA", "R"), ("AGG", "R"),
    ("CTA", "L"), ("CTC", "L"), ("CTG", "L"), ("CTT", "L"),
    ("CCA", "P"), ("CCC", "P"), ("CCG", "P"), ("CCT", "P"),
    ("CAC", "H"), ("CAT", "H"), ("CAA", "Q"), ("CAG", "Q"),
    ("CGA", "R"), ("CGC", "R"), ("CGG", "R"), ("CGT", "R"),
    ("GTA", "V"), ("GTC", "V"), ("GTG", "V"), ("GTT", "V"),
    ("GCA", "A"), ("GCC", "A"), ("GCG", "A"), ("GCT", "A"),
    ("GAC", "D"), ("GAT", "D"), ("GAA", "E"), ("GAG", "E"),
    ("GGA", "G"), ("GGC", "G"), ("GGG", "G"), ("GGT", "G"),
    ("TCA", "S"), ("TCC", "S"), ("TCG", "S"), ("TCT", "S"),
    ("TTC", "F"), ("TTT", "F"), ("TTA", "L"), ("TTG", "L"),
    ("TAC", "Y"), ("TAT", "Y"), ("TAA", "_"), ("TAG", "_"),
    ("TGC", "C"), ("TGT", "C"), ("TGA", "_"), ("TGG", "W"),
)
CODON_TABLE = dict(CODON_PAIRS)


def clear_screen():
    input("\nNacisnij Enter, aby kontynuowac...")
    os.system("cls" if os.name == "nt" else "clear")


def show_menu():
    print("Menu:")
    print("1. Oblicz GC-content")
    print("2. Policz czestotliwosci wystepowania zasad")
    print("3. Generuj dotplot (macierz podobienstwa)")
    print("4. Czestotliwosc aminokwasow z pliku PDB")
    print("5. Statystyki kodonow dla aminokwasow")
    print("6. Oblicz mape kontaktow (bialko/RNA z PDB)")
    print("0. Zamknij program")


def calc_gc():
    dna_seq = input("Podaj sekwencje DNA: ").upper().replace(" ", "")
    if not dna_seq:
        print("Brak danych do obliczen.")
        return
    gc_percent = (dna_seq.count("G") + dna_seq.count("C")) / len(dna_seq) * 100
    print(f"Wynik: {gc_percent:.2f}%")


def count_bases():
    sequences = input("Wpisz sekwencje oddzielone spacjami: ").upper().split()
    if not sequences:
        print("Brak sekwencji.")
        return
    header = f"{'Zasada':<8}| " + " | ".join(
        [f"Sekw {i + 1:<2}" for i in range(len(sequences))]
    )
    print("\n" + header + "\n" + "-" * len(header))
    for base in "ACGT":
        row = f" {base:<6}| " + " | ".join(
            [f"{seq.count(base):^6}" for seq in sequences]
        )
        print(row)


def make_dotplot():
    seq1 = input("Sekwencja 1: ").upper().replace(" ", "")
    seq2 = input("Sekwencja 2: ").upper().replace(" ", "")
    if not seq1 or not seq2:
        print("Brak danych do porownania.")
        return
    print("\n " + " ".join(seq2))
    for char1 in seq1:
        row = char1 + " " + " ".join(
            "*" if char1 == char2 else "." for char2 in seq2
        )
        print(row)


def count_amino_acids():
    pdb_path = input("Podaj nazwe pliku PDB (np. example.pdb): ").strip()
    if not pdb_path:
        print("Brak nazwy pliku.")
        return
    aa_counts = {}
    seen_residues = set()
    try:
        with open(pdb_path, "r") as handle:
            for line in handle:
                if not line.startswith("ATOM"):
                    continue
                aa = line[17:20].strip()
                chain = line[21].strip()
                resid = line[22:26].strip()
                key = (resid, chain)
                if key in seen_residues:
                    continue
                seen_residues.add(key)
                aa_counts[aa] = aa_counts.get(aa, 0) + 1
        print(f"\nWyniki dla {pdb_path}:")
        for aa, count in sorted(aa_counts.items()):
            print(f"{aa}: {count}")
    except FileNotFoundError:
        print("Blad: nie znaleziono pliku.")
    except OSError as exc:
        print(f"Blad: {exc}")


def codon_stats():
    coding_seq = (
        input("Wklej sekwencje kodujaca: ").upper().replace("U", "T").replace(" ", "")
    )
    if len(coding_seq) < 3:
        print("Za krotka sekwencja.")
        return
    codon_stats = {}
    for i in range(0, len(coding_seq) - 2, 3):
        codon = coding_seq[i : i + 3]
        aa = CODON_TABLE.get(codon, "Nieznany")
        codon_stats.setdefault(aa, {})
        codon_stats[aa][codon] = codon_stats[aa].get(codon, 0) + 1
    for aa, codons in codon_stats.items():
        info = ", ".join([f"{cod}: {count}" for cod, count in codons.items()])
        print(f"AA {aa} -> {info}")


def contact_map():
    pdb_path = input("Podaj nazwe pliku PDB (example.pbd): ").strip()
    if not pdb_path:
        print("Brak nazwy pliku.")
        return
    coordinates = []
    try:
        with open(pdb_path, "r") as handle:
            for line in handle:
                if not line.startswith("ATOM"):
                    continue
                atom_name = line[12:16].strip()
                if atom_name not in ("CA", "P"):
                    continue
                x = float(line[30:38])
                y = float(line[38:46])
                z = float(line[46:54])
                coordinates.append((x, y, z))
        if not coordinates:
            print("Brak danych do mapy kontaktow.")
            return
        threshold = 8.0
        print(f"\nMapa kontaktow (prog {threshold}A):")
        for coord_a in coordinates:
            row = []
            for coord_b in coordinates:
                dist = math.sqrt(sum((a - b) ** 2 for a, b in zip(coord_a, coord_b)))
                row.append("#" if dist < threshold else ".")
            print(" ".join(row))
    except FileNotFoundError:
        print("Blad: nie znaleziono pliku.")
    except ValueError as exc:
        print(f"Blad danych: {exc}")
    except OSError as exc:
        print(f"Blad: {exc}")


def main():
    actions = {
        "1": calc_gc,
        "2": count_bases,
        "3": make_dotplot,
        "4": count_amino_acids,
        "5": codon_stats,
        "6": contact_map,
    }
    while True:
        show_menu()
        choice = input("Wybierz opcje (1-6): ").strip()
        if choice == "0":
            print("Koniec programu.")
            break
        action = actions.get(choice)
        if not action:
            print("Nieprawidlowy wybor, sprobuj ponownie.")
            continue
        action()
        clear_screen()


if __name__ == "__main__":
    main()
