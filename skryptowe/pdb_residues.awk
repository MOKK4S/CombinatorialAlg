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
