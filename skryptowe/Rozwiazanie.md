Zadanie 1
wget -O 1FNT.pdb https://files.rcsb.org/download/1FNT.pdb && wget -O 3L5Q.pdb https://files.rcsb.org/download/3L5Q.pdb && wget -O 1FNT.fasta https://www.rcsb.org/fasta/entry/1FNT && wget -O 4V7O.fasta https://www.rcsb.org/fasta/entry/4V7O && less 1FNT.pdb 3L5Q.pdb 1FNT.fasta 4V7O.fasta

Zadanie 2
more -10 1FNT.pdb

Zadanie 3
cat 1FNT.fasta - 4V7O.fasta > moje_sekwencje

Zadanie 4
find ~ -maxdepth 1 -type f -exec head -n 5 -q {} +

Zadanie 5
sed -n '3,5p' 1FNT.pdb

Zadanie 6
tail -n 7 1FNT.fasta | head -n 3

Zadanie 7
tr -d '\n' < /etc/passwd

Zadanie 8
tr -s '[:space:]' '\n' < tekst.txt > tekst_slowami.txt

Zadanie 9
find /etc -type f | wc -l

Zadanie 10
head -n 3 /etc/passwd | wc -c

Zadanie 11
ls | tr '[:lower:]' '[:upper:]'

Zadanie 12
ls -l | awk 'NR>1 {print $1, $5, $9}'

Zadanie 13
ls -l | tail -n +2 | sort -k5,5n

Zadanie 14
sort -t: -k3,3nr /etc/passwd

Zadanie 15
sort -t: -k4,4nr -k3,3nr /etc/passwd

Zadanie 16
find /home -type f -printf '%u\n' | sort | uniq -c

Zadanie 17
find . -printf '%M\n' | sort | uniq -c

Zadanie 18
kill 1234 1>killout.txt 2>killerr.txt

Zadanie 19
kill 1234 2>&1

Zadanie 20
echo 'TTTTTAAAGAAAAGAAAAAATAATCCAAACTCCTCTTCCTCATAAGAC' | tr 'ATCGatcg' 'TAGCtagc' | rev

Zadanie 21
cat <<'SCRIPT' > pdb_to_sequence.sh
#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <structure.pdb|structure.cif>" >&2
  exit 1
fi

python3 - "$1" <<'PY'
import sys, shlex, collections

aa_map = {
    'ALA': 'A', 'ARG': 'R', 'ASN': 'N', 'ASP': 'D', 'CYS': 'C', 'GLN': 'Q', 'GLU': 'E', 'GLY': 'G',
    'HIS': 'H', 'ILE': 'I', 'LEU': 'L', 'LYS': 'K', 'MET': 'M', 'PHE': 'F', 'PRO': 'P', 'SER': 'S',
    'THR': 'T', 'TRP': 'W', 'TYR': 'Y', 'VAL': 'V', 'SEC': 'U', 'PYL': 'O', 'ASX': 'B', 'GLX': 'Z',
    'UNK': 'X'
}

def aa3to1(code):
    return aa_map.get(code.upper(), '')

def process_pdb(path):
    seqs = collections.OrderedDict()
    seen = set()
    with open(path, 'r', encoding='utf-8', errors='ignore') as fh:
        for line in fh:
            if not line.startswith(('ATOM', 'HETATM')):
                continue
            resname = line[17:20].strip()
            chain = line[21].strip() or '-'
            resnum = line[22:26].strip()
            icode = line[26].strip()
            key = (chain, resnum, icode)
            if key in seen:
                continue
            seen.add(key)
            aa = aa3to1(resname)
            if not aa:
                continue
            seqs.setdefault(chain, []).append(aa)
    return seqs

def process_mmcif(path):
    seqs = collections.OrderedDict()
    seen = set()
    with open(path, 'r', encoding='utf-8', errors='ignore') as fh:
        columns = []
        collecting = False
        data_rows = []

        def flush():
            nonlocal seqs, seen, data_rows, columns, collecting
            if not collecting or not columns:
                data_rows = []
                return
            try:
                comp_idx = columns.index('_atom_site.label_comp_id')
            except ValueError:
                data_rows = []
                return
            try:
                chain_idx = columns.index('_atom_site.auth_asym_id')
            except ValueError:
                try:
                    chain_idx = columns.index('_atom_site.label_asym_id')
                except ValueError:
                    data_rows = []
                    return
            try:
                res_idx = columns.index('_atom_site.auth_seq_id')
            except ValueError:
                try:
                    res_idx = columns.index('_atom_site.label_seq_id')
                except ValueError:
                    data_rows = []
                    return
            ins_idx = columns.index('_atom_site.pdbx_PDB_ins_code') if '_atom_site.pdbx_PDB_ins_code' in columns else None
            for row in data_rows:
                try:
                    tokens = shlex.split(row)
                except ValueError:
                    continue
                if len(tokens) < len(columns):
                    continue
                resname = tokens[comp_idx]
                chain = tokens[chain_idx]
                resnum = tokens[res_idx]
                icode = tokens[ins_idx] if ins_idx is not None else ''
                chain = chain if chain != '?' else '-'
                key = (chain, resnum, icode)
                if key in seen:
                    continue
                seen.add(key)
                aa = aa3to1(resname)
                if not aa:
                    continue
                seqs.setdefault(chain, []).append(aa)
            data_rows = []

        for raw_line in fh:
            stripped = raw_line.strip()
            if not stripped:
                continue
            if stripped.lower() == 'loop_':
                flush()
                collecting = True
                columns = []
                data_rows = []
                continue
            if collecting and stripped.startswith('_'):
                columns.append(stripped)
                continue
            if stripped.startswith('#'):
                flush()
                collecting = False
                columns = []
                data_rows = []
                continue
            if collecting and columns and not stripped.startswith('_'):
                data_rows.append(stripped)
        flush()
    return seqs

path = sys.argv[1]
if path.lower().endswith(('.pdb', '.ent')):
    sequences = process_pdb(path)
else:
    sequences = process_mmcif(path)

if not sequences:
    sys.exit('No ATOM records found.')

for chain, letters in sequences.items():
    print('Chain {}: {}'.format(chain, ''.join(letters)))
PY
SCRIPT
chmod +x pdb_to_sequence.sh
