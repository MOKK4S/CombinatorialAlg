#!/usr/bin/env python3
from collections import Counter, defaultdict
from pathlib import Path

from Bio.Data import CodonTable
from Bio.PDB import PDBParser
from Bio.PDB.Polypeptide import is_aa


BASE_DIR = Path(__file__).resolve().parent


def read_seq(name):
    path = BASE_DIR / Path(name).name
    try:
        text = path.read_text(encoding="utf-8", errors="ignore")
    except FileNotFoundError:
        print(f"Brak pliku: {path.name}")
        return ""
    seq = "".join(ch.upper() for ch in text if ch.isalpha())
    if not seq:
        print(f"Brak sekwencji w pliku: {path.name}")
    return seq


def read_list(prompt):
    raw = input(prompt).strip()
    if not raw:
        print("Brak plikow.")
        return []
    result = []
    for name in raw.split():
        seq = read_seq(name)
        if seq:
            result.append((name, seq))
    return result


def task_gc():
    records = read_list("Podaj pliki z sekwencja (np. przyklad.txt): ")
    if not records:
        return
    print("plik\tgc_percent\tlength")
    for name, seq in records:
        dna = "".join(ch for ch in seq if ch in "ACGT")
        length = len(dna)
        gc = (dna.count("G") + dna.count("C")) / length if length else 0.0
        print(f"{name}\t{gc * 100:.2f}\t{length}")


def task_basefreq():
    records = read_list("Podaj pliki z sekwencja (np. a.txt b.txt): ")
    if not records:
        return
    bases = ["A", "C", "G", "T"]
    table = {}
    for name, seq in records:
        seq = seq.upper()
        counts = Counter(base for base in seq if base in bases)
        total = sum(counts.values()) or 1
        table[name] = {base: counts.get(base, 0) / total for base in bases}

    header = ["base"] + [name for name, _ in records]
    print("\t".join(header))
    for base in bases:
        row = [base]
        for name, _ in records:
            row.append(f"{table[name][base]:.3f}")
        print("\t".join(row))


def task_dotplot():
    name1 = input("Podaj plik dla sekwencji 1: ").strip()
    name2 = input("Podaj plik dla sekwencji 2: ").strip()
    seq1 = read_seq(name1)
    seq2 = read_seq(name2)
    if not seq1 or not seq2:
        return
    for ch1 in seq1:
        line = ["1" if ch1 == ch2 else "0" for ch2 in seq2]
        print("".join(line))


def task_pdbfreq():
    name = input("Podaj plik PDB: ").strip()
    path = BASE_DIR / Path(name).name
    if not path.exists():
        print(f"Brak pliku: {path.name}")
        return
    model_input = input("Model id (pusty = pierwszy): ").strip()
    chain_id = input("Lancuch (pusty = wszystkie): ").strip()
    model_id = int(model_input) if model_input else None

    parser = PDBParser(QUIET=True)
    structure = parser.get_structure("struct", str(path))
    model = next(structure.get_models()) if model_id is None else structure[model_id]

    residues = []
    if chain_id:
        try:
            residues = list(model[chain_id].get_residues())
        except KeyError:
            print(f"Nie znaleziono lancucha {chain_id}.")
            return
    else:
        for chain in model:
            residues.extend(chain.get_residues())

    counts = Counter()
    for residue in residues:
        if is_aa(residue, standard=True):
            counts[residue.get_resname()] += 1

    total = sum(counts.values()) or 1
    print("residue\tcount\tfrequency")
    for resname in sorted(counts):
        count = counts[resname]
        print(f"{resname}\t{count}\t{count / total:.3f}")


def task_codonstats():
    records = read_list("Podaj pliki z sekwencja kodujaca: ")
    if not records:
        return

    table = CodonTable.unambiguous_dna_by_id[1]
    codon_to_aa = dict(table.forward_table)
    codon_to_aa.update({codon: "*" for codon in table.stop_codons})

    codon_counts = Counter()
    for _, seq in records:
        seq = seq.upper().replace("U", "T")
        for i in range(0, len(seq) - 2, 3):
            codon = seq[i : i + 3]
            if any(ch not in "ACGT" for ch in codon):
                continue
            if codon in codon_to_aa:
                codon_counts[codon] += 1

    aa_to_codons = defaultdict(list)
    for codon, aa in codon_to_aa.items():
        aa_to_codons[aa].append(codon)

    print("aa\ttotal\tcodons")
    for aa in sorted(aa_to_codons):
        codons = sorted(aa_to_codons[aa])
        total = sum(codon_counts.get(codon, 0) for codon in codons)
        parts = " ".join(f"{codon}:{codon_counts.get(codon, 0)}" for codon in codons)
        print(f"{aa}\t{total}\t{parts}")


def task_contacts():
    name = input("Podaj plik PDB: ").strip()
    path = BASE_DIR / Path(name).name
    if not path.exists():
        print(f"Brak pliku: {path.name}")
        return
    model_input = input("Model id (pusty = pierwszy): ").strip()
    chain_id = input("Lancuch (np. A): ").strip()
    if not chain_id:
        print("Brak lancucha.")
        return
    type_input = input("Typ lancucha (protein/rna) [protein]: ").strip().lower()
    if not type_input:
        type_input = "protein"
    threshold_input = input("Prog odleglosci [8.0]: ").strip()
    threshold = float(threshold_input) if threshold_input else 8.0
    model_id = int(model_input) if model_input else None

    parser = PDBParser(QUIET=True)
    structure = parser.get_structure("struct", str(path))
    model = next(structure.get_models()) if model_id is None else structure[model_id]
    try:
        chain = model[chain_id]
    except KeyError:
        print(f"Nie znaleziono lancucha {chain_id}.")
        return

    atom_name = "CA" if type_input != "rna" else "P"
    require_aa = type_input != "rna"
    coords = []
    for residue in chain.get_residues():
        if require_aa and not is_aa(residue, standard=True):
            continue
        if atom_name in residue:
            coords.append(residue[atom_name].get_coord())

    if not coords:
        print(f"Nie znaleziono atomow {atom_name} w lancuchu.")
        return

    threshold_sq = threshold * threshold
    for coord_i in coords:
        line = []
        for coord_j in coords:
            diff = coord_i - coord_j
            dist_sq = diff[0] ** 2 + diff[1] ** 2 + diff[2] ** 2
            line.append("1" if dist_sq <= threshold_sq else "0")
        print("".join(line))


def run_all():
    print("=== 1 GC-content ===")
    task_gc()
    print("\n=== 2 Base frequencies ===")
    task_basefreq()
    print("\n=== 3 Dotplot ===")
    task_dotplot()
    print("\n=== 4 PDB amino acid frequencies ===")
    task_pdbfreq()
    print("\n=== 5 Codon/AA stats ===")
    task_codonstats()
    print("\n=== 6 Contact map ===")
    task_contacts()


def main():
    print("Wybierz zadanie:")
    print("1 - GC-content")
    print("2 - Czestotliwosci zasad A,C,G,T")
    print("3 - Dotplot dla dwoch sekwencji")
    print("4 - Czestotliwosc aminokwasow w PDB")
    print("5 - Statystyki kodonow i aminokwasow")
    print("6 - Mapa kontaktow (bialko lub RNA)")
    print("7 - Wszystko")
    print("0 - Wyjscie")

    choice = input("Twoj wybor: ").strip()
    if choice == "1":
        task_gc()
    elif choice == "2":
        task_basefreq()
    elif choice == "3":
        task_dotplot()
    elif choice == "4":
        task_pdbfreq()
    elif choice == "5":
        task_codonstats()
    elif choice == "6":
        task_contacts()
    elif choice == "7":
        run_all()
    else:
        print("Koniec.")


if __name__ == "__main__":
    main()
