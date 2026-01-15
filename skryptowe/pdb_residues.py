#!/usr/bin/env python3
import sys
from collections import OrderedDict

path = sys.argv[1] if len(sys.argv) > 1 else ""
if not path:
    print("Usage: pdb_residues.py <file.pdb>")
    sys.exit(1)

model = "1"
had = False
seqs = OrderedDict()
seen = set()

with open(path, "r", encoding="utf-8", errors="ignore") as fh:
    for line in fh:
        if line.startswith("MODEL"):
            model = line[10:14].strip() or "1"
            had = True
            continue
        if not line.startswith("ATOM"):
            continue
        chain = line[21].strip() or "-"
        resnum = line[22:26].strip()
        resname = line[17:20].strip()
        key = (model, chain, resnum)
        if key in seen:
            continue
        seen.add(key)
        seqs.setdefault((model, chain), []).append(resname)

if had:
    current = None
    for (m, chain), residues in seqs.items():
        if m != current:
            print(f"Model {m}:")
            current = m
        print(f"  Chain {chain}: {' '.join(residues)}")
else:
    for (_, chain), residues in seqs.items():
        print(f"Chain {chain}: {' '.join(residues)}")
