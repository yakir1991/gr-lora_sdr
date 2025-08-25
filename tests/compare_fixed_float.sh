#!/bin/sh
set -e
if cmp --silent "$1" "$2"; then
  echo "Fixed-point and floating-point outputs match"
  exit 0
else
  echo "Fixed-point and floating-point outputs differ"
  exit 1
fi
