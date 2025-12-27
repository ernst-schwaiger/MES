#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <c-filename> <manifest-filename>"
    exit 1
fi

FILE=$1
MANIFEST=$2
DATE=$(date +"%y%m%d-%H:%M")

{
  echo "__attribute__((section(\".hellofunc\")))"
  echo "void hellofunc(char *buf)"
  echo "{"

  for (( i=0; i<${#MSG}; i++ )); do
    echo -n "    buf[$i] = '"
    echo -n "${MSG:$i:1}"
    echo "';"
  done
    echo "    buf[${#MSG}] = '\n';"
    echo "    buf[$((${#MSG} + 1))] = '\0';"

  echo "}"
}  >> "${FILE}"

{
  echo "SRC_FILE=${FILE}"
  echo "VERSION=${VERSION}"
  echo "DATE=${DATE}"
} >> "${MANIFEST}"

