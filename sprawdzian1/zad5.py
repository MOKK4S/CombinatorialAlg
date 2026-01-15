#!/usr/bin/python3
#IMIE i NAZWISKO: 
#Numer indeksu:
INPUT="""
Mikołaj, Mikołaj jedzie samochodem
bo gdzieś zgubił saneczki w tę mroźną pogodę.
Ref. U - chu . cha, tra . la . la
Co to za Mikołaj?
U - chu . cha, tra . la . la
Co to za Mikołaj?
"""
OUTPUT="al . al . art ,ahc . uhc - U"
print(INPUT)
print("\n\n")

######     Kod do modyfikacji     #####
#Zadanie: Zmodyfikuj kod we wskazanych poniżej miejscach tak,
#aby zawartość zmiennej wynik była równa:
#"al . al . art ,ahc . uhc - U"
#Uwaga: konstrukcja wynik="al . al . art ,ahc . uhc - U" nie
#jest dopuszczalnym rozwiązaniem
tmp=INPUT.splitlines()
wynik = tmp[3][5:][::-1] 

###### Koniec kodu do modyfikacji #####
if (wynik == OUTPUT):
  print("{0}\nTest 1: OK\n".format(wynik))
else:
  print("Twój wynik:\n{0} \nTest 1: ERROR\n".format(wynik))
  print( "Prawidłowy wynik to: {0}\n".format(OUTPUT))
