Zadanie 1
3GAU: grep '^ATOM' skryptowe/3GAU.pdb | wc -l -> 9704
2KYV: grep '^ATOM' skryptowe/2KYV.pdb | wc -l -> 89100

Zadanie 2
3GAU: grep '^MODEL' skryptowe/3GAU.pdb | wc -l -> 8 modeli
2KYV: grep '^MODEL' skryptowe/2KYV.pdb | wc -l -> 20 modeli

Zadanie 3
3GAU:
Model 1: A
Model 2: A
Model 3: A
Model 4: A
Model 5: A
Model 6: A
Model 7: A
Model 8: A

2KYV:
Model 1: A B C D E
Model 2: A B C D E
Model 3: A B C D E
Model 4: A B C D E
Model 5: A B C D E
Model 6: A B C D E
Model 7: A B C D E
Model 8: A B C D E
Model 9: A B C D E
Model 10: A B C D E
Model 11: A B C D E
Model 12: A B C D E
Model 13: A B C D E
Model 14: A B C D E
Model 15: A B C D E
Model 16: A B C D E
Model 17: A B C D E
Model 18: A B C D E
Model 19: A B C D E
Model 20: A B C D E

Zadanie 4
sed 's/ HIS / HSD /g' skryptowe/3GAU.pdb > skryptowe/3GAU_hsd.pdb
sed 's/ HSD / HIS /g' skryptowe/3GAU_hsd.pdb > skryptowe/3GAU_his.pdb
sed 's/ HIS / HSD /g' skryptowe/2KYV.pdb > skryptowe/2KYV_hsd.pdb
sed 's/ HSD / HIS /g' skryptowe/2KYV_hsd.pdb > skryptowe/2KYV_his.pdb

Zadanie 5
Poniżej skrypty wypisujace identyfikatory aminokwasow w notacji trzyliterowej,
z podzialem na lancuchy i modele (gdy rekordy MODEL sa obecne).

Bash
```bash
#!/usr/bin/env bash
file=$1
model=1
had_model=0
declare -A seq seen pair_seen printed
pairs=()

while IFS= read -r line; do
  if [[ ${line:0:5} == "MODEL" ]]; then
    model=${line:10:4}
    model=${model//[[:space:]]/}
    model=${model:-1}
    had_model=1
    continue
  fi

  [[ ${line:0:4} != "ATOM" ]] && continue

  chain=${line:21:1}
  [[ -z $chain ]] && chain="-"

  resnum=${line:22:4}
  resnum=${resnum//[[:space:]]/}
  resname=${line:17:3}

  key="$model|$chain|$resnum"
  [[ ${seen[$key]+x} ]] && continue
  seen[$key]=1

  pair="$model|$chain"
  if [[ -z ${pair_seen[$pair]+x} ]]; then
    pair_seen[$pair]=1
    pairs+=("$pair")
  fi

  seq[$pair]="${seq[$pair]}${resname} "
done < "$file"

if [[ $had_model -eq 0 ]]; then
  for pair in "${pairs[@]}"; do
    IFS="|" read -r _ chain <<< "$pair"
    s=${seq[$pair]}
    s=${s%% }
    echo "Chain $chain: $s"
  done
else
  for pair in "${pairs[@]}"; do
    IFS="|" read -r m chain <<< "$pair"
    if [[ -z ${printed[$m]+x} ]]; then
      printed[$m]=1
      echo "Model $m:"
    fi
    s=${seq[$pair]}
    s=${s%% }
    echo "  Chain $chain: $s"
  done
fi
```

Awk
```awk
#!/usr/bin/awk -f
BEGIN { model = 1 }

/^MODEL/ {
  model = $2
  had = 1
  next
}

/^ATOM/ {
  chain = substr($0, 22, 1)
  if (chain == "") chain = "-"
  resnum = substr($0, 23, 4)
  gsub(/ /, "", resnum)
  resname = substr($0, 18, 3)
  key = model SUBSEP chain SUBSEP resnum
  if (seen[key]++) next
  pair = model SUBSEP chain
  seq[pair] = seq[pair] resname " "
  if (!(pair in order)) list[++n] = pair
}

END {
  for (i = 1; i <= n; i++) {
    split(list[i], a, SUBSEP)
    m = a[1]
    c = a[2]
    s = seq[list[i]]
    sub(/[[:space:]]+$/, "", s)
    if (had) {
      if (!printed[m]++) printf("Model %s:\n", m)
      printf("  Chain %s: %s\n", c, s)
    } else {
      printf("Chain %s: %s\n", c, s)
    }
  }
}
```

Python (opcjonalnie)
```python
#!/usr/bin/env python3
import sys
from collections import OrderedDict

path = sys.argv[1]
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
```

Aby wypisywac kody jednoliterowe, dodaj mapowanie resname -> litera i wstawiaj litery
zamiast kodow trzyliterowych.
