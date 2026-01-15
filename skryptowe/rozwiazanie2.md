## Zadanie 1
wget https://files.rcsb.org/download/1FNT.pdb
wget https://files.rcsb.org/download/3L5Q.pdb
wget https://www.rcsb.org/fasta/entry/1FNT -O 1FNT.fasta
wget https://www.rcsb.org/fasta/entry/4V7O -O 4V7O.fasta
less 1FNT.pdb 3L5Q.pdb 1FNT.fasta 4V7O.fasta

## Zadanie 2
  more -10 1FNT.pdb

## Zadanie 3
cat 1FNT.fasta - 4V7O.fasta > moje_sekwencje

## Zadanie 4
find ~ -maxdepth 1 -type f -exec head -n 5 -q {} \;

## Zadanie 5
sed -n '3,5p' 1FNT.pdb

## Zadanie 6
tail -n 7 1FNT.fasta | head -n 3

  ## Zadanie 7
  tr -d '\n' < /etc/passwd

## Zadanie 8
tr -s '[:space:]' '\n' < tekst.txt > tekst_slowami.txt

## Zadanie 9
find /etc -type f | wc -l

## Zadanie 10
head -n 3 /etc/passwd | wc -c

## Zadanie 11
ls | tr '[:lower:]' '[:upper:]'

## Zadanie 12
ls -l | awk '{print $1, $5, $9}'

## Zadanie 13
ls -lS

## Zadanie 14
sort -t: -k3,3nr /etc/passwd

## Zadanie 15
sort -t: -k4,4nr -k3,3nr /etc/passwd

## Zadanie 16
find /home -type f -printf '%u\n' | sort | uniq -c

## Zadanie 17
find . -printf '%M\n' | sort | uniq -c

## Zadanie 18
kill 1234 > killout.txt 2> killerr.txt

## Zadanie 19
kill 1234 2>&1

## Zadanie 20
echo 'TTTTTAAAGAAAAGAAAAAATAATCCAAACTCCTCTTCCTCATAAGAC' | tr 'ATCGatcg' 'TAGCtagc' | rev

## Zadanie 21
cat > pdb_to_sequence.sh <<'SCRIPT'
#!/bin/bash
awk '
BEGIN {
  map["ALA"]="A"; map["ARG"]="R"; map["ASN"]="N"; map["ASP"]="D"; map["CYS"]="C";
  map["GLN"]="Q"; map["GLU"]="E"; map["GLY"]="G"; map["HIS"]="H"; map["ILE"]="I";
  map["LEU"]="L"; map["LYS"]="K"; map["MET"]="M"; map["PHE"]="F"; map["PRO"]="P";
  map["SER"]="S"; map["THR"]="T"; map["TRP"]="W"; map["TYR"]="Y"; map["VAL"]="V";
}
/^ATOM/ {
  chain = substr($0,22,1);
  resid = substr($0,23,4);
  key = chain resid;
  if (!(key in seen)) {
    seen[key]=1;
    aa = map[substr($0,18,3)];
    if (aa != "") seq[chain] = seq[chain] aa;
  }
}
END {
  for (c in seq) print "Chain " c ": " seq[c];
}
' "$1"
SCRIPT
chmod +x pdb_to_sequence.sh
