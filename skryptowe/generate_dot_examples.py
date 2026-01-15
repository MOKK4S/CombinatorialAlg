#!/usr/bin/env python3
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
EXERCISE_DIR = ROOT / "Exercise2"
EXE_PATH = ROOT / "build" / "exercise2"

# Przykłady użyte w sprawozdaniu
EXAMPLES = [1, 2, 3, 11]


def read_graph(path: Path):
    with path.open() as f:
        header = f.readline().strip().split()
        if len(header) != 2:
            raise ValueError(f"Invalid header in {path}")
        n = int(header[0])
        m = int(header[1])
        edges = []
        for _ in range(m):
            line = f.readline()
            if not line:
                break
            u_str, v_str = line.strip().split()
            edges.append((int(u_str), int(v_str)))
    return n, edges


def write_dot(path: Path, name: str, n: int, edges):
    with path.open("w") as f:
        f.write(f"digraph {name} {{\n")
        # Wierzchołki 1..n, żeby pokazać także izolowane (gdyby się zdarzyły)
        for v in range(1, n + 1):
            f.write(f"    {v};\n")
        for u, v in edges:
            f.write(f"    {u} -> {v};\n")
        f.write("}\n")


def main():
    if not EXE_PATH.exists():
        msg = (
            f"Nie znaleziono pliku wykonywalnego {EXE_PATH}. "
            "Najpierw skompiluj program, np.: "
            "g++ -std=c++20 -Wall -Wextra -pedantic Exercise2/Exercise2.cpp -o build/exercise2"
        )
        raise SystemExit(msg)

    dot_dir = EXERCISE_DIR / "dot"
    dot_dir.mkdir(exist_ok=True)

    for n in EXAMPLES:
        g_path = EXERCISE_DIR / f"graph{n}.txt"
        h_path = EXERCISE_DIR / f"graph_out{n}.txt"

        # Uruchamiamy program, żeby wygenerować graf H dla danego G
        subprocess.run(
            [str(EXE_PATH), str(g_path), str(h_path)],
            check=True,
            cwd=ROOT,
        )

        g_n, g_edges = read_graph(g_path)
        h_n, h_edges = read_graph(h_path)

        g_dot = dot_dir / f"graph{n}_G.dot"
        h_dot = dot_dir / f"graph{n}_H.dot"

        write_dot(g_dot, f"graph{n}_G", g_n, g_edges)
        write_dot(h_dot, f"graph{n}_H", h_n, h_edges)

        print(f"Wygenerowano {g_dot} i {h_dot}")


if __name__ == "__main__":
    main()
