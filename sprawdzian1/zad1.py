# W podanym niżej łańcuchu znajdź i wypisz powtarzający się motyw
# o długość 4 znaków zaczynający się od A
# i powtarzający się co najmniej 5 razy
# (powtórzenia mogą być oddzielone innymi podsekwencjami)
import re

s = "CCACACAGGTAATGAATGAATCGAATGAATGAATGCCTAAGTGCC"
r = re.search(r"(A...)(?:.*\1){4}", s).group(1)
print(r)
