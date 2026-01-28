import re

DIGIT_WORDS = {
    "0": "zero",
    "1": "jeden",
    "2": "dwa",
    "3": "trzy",
    "4": "cztery",
    "5": "piec",
    "6": "szesc",
    "7": "siedem",
    "8": "osiem",
    "9": "dziewiec",
}


def show_menu():
    print("Menu:")
    print("1. Szukaj wyrazenia w tekscie i zlicz wystapienia")
    print("2. Zamien cyfry na slowa")
    print("3. Zaznacz powtorzenia slow w nawiasach")
    print("4. Usun powtorzenia slow")
    print("5. Przyklady regex dla DNA")
    print("0. Zakoncz")


def find_regex_in_text():
    text = input("Wklej tekst: ")
    pattern = input("Podaj wyrazenie regularne: ").strip()
    matches = re.findall(pattern, text)
    print(f"Liczba wystapien: {len(matches)}")
    print("Przyklady wyrazen:")
    print("Liczby pieciocyfrowe: \\b\\d{5}\\b")
    print("A + znak inny niz x,y,z: A[^xyz]")
    print("Same spacje: ^\\s+$")


def digits_to_words():
    text = input("Podaj tekst: ")
    result = re.sub(r"\d", lambda m: DIGIT_WORDS[m.group(0)], text)
    print(f"Wynik: {result}")


def bracket_repeats():
    text = input("Podaj tekst: ")
    pattern = r"\b(\w+)(?:\s+\1)+\b"
    result = re.sub(pattern, lambda m: f"[{m.group(0)}]", text)
    print(f"Wynik: {result}")


def remove_repeats():
    text = input("Podaj tekst: ")
    pattern = r"\b(?P<word>\w+)(?:\s+(?P=word))+\b"
    result = re.sub(pattern, r"\g<word>", text)
    print(f"Wynik: {result}")


def test_dna_regex():
    dna = input("Podaj sekwencje DNA: ").upper().replace(" ", "")
    patterns = {
        "Start ATG": r"ATG",
        "Motyw A[CT]G": r"A[CT]G",
        "Powtorzenia AT": r"(?:AT){2,}",
        "ORF": r"ATG(?:...)*?(?:TAA|TAG|TGA)",
    }
    print("Wyniki dopasowan:")
    for label, pattern in patterns.items():
        matches = re.findall(pattern, dna)
        print(f"{label} ({pattern}) -> {matches}")


def main():
    actions = {
        "1": find_regex_in_text,
        "2": digits_to_words,
        "3": bracket_repeats,
        "4": remove_repeats,
        "5": test_dna_regex,
    }
    while True:
        show_menu()
        choice = input("Wybierz opcje: ").strip()
        if choice == "0":
            print("Koniec.")
            break
        action = actions.get(choice)
        if action:
            action()
        else:
            print("Zly wybor.")


if __name__ == "__main__":
    main()
