#!/usr/bin/env python3
from itertools import combinations
from pathlib import Path
import random


def build_points(segments):
    pts = [0]
    s = 0
    for d in segments:
        s += d
        pts.append(s)
    return pts


def build_A(points):
    return sorted(b - a for a, b in combinations(points, 2))


def main():
    raw = input("Podaj liczbe odcinkow (dlugosc mapy): ").strip()
    if not raw:
        raise SystemExit("Brak danych.")
    n = int(raw)
    if n <= 0:
        raise SystemExit("Liczba odcinkow musi byc dodatnia.")

    segments = [random.randint(1, 10) for _ in range(n)]
    points = build_points(segments)
    A = build_A(points)

    out_path = Path(__file__).with_name("data") / "generated_auto.txt"
    out_path.parent.mkdir(exist_ok=True)
    out_path.write_text(" ".join(map(str, A)) + "\n", encoding="ascii")

    print("Mapa (odcinki):", " ".join(map(str, segments)))
    print("L:", points[-1])
    print("Zapisano:", out_path)


if __name__ == "__main__":
    main()
