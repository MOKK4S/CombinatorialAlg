###########Kod do modyfikacji###########
#Zaimplementuj dekorator nagroda, który 
#zmodyfikuj działanie funkcji wyplata
#tak, że wynik będzie wyższy 2 razy (*2) 
#od pierwotnego

########################################
def nagroda(funkcja):
    def wrapper(*args, **kwargs):
        return 2 * funkcja(*args, **kwargs)

    return wrapper
########################################
@nagroda
def wyplata(pensja):
    return 0.7*pensja

#Koniec kodu do modyfikacji#
if (wyplata(100)==70):
    print("Niewłaściwa definicja dekoratora")
elif (wyplata(100)==140):
    print("Wynik prawidłowy")
else:
    print("Wynik nieprawidłowy wypłacono pensję w wysokości {0}".format(wyplata(100)))
