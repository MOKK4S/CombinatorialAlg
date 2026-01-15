#!/usr/bin/env bash
file=$1
if [[ -z $file ]]; then
  echo "<file.pdb>" >&2
  exit 1
fi

model=1
had_model=0
declare -A seen seq pair_seen printed
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
