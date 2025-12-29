#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <c-filename> <manifest-filename>"
    exit 1
fi

FILE=$1
MANIFEST=$2
DATE=$(date +"%y%m%d-%H:%M")

true > "${FILE}"
{
  echo "__attribute__((section(\".hellofunc\")))"
  echo "void hellofunc(char *buf)"
  echo "{"

  for (( i=0; i<${#MSG}; i++ )); do
    printf "    buf[%d] = '%c';\n" "$i" "${MSG:$i:1}"
  done

  printf "    buf[%d] = '\\\\n';\n" "${#MSG}"
  printf "    buf[%d] = '\\\\0';\n" "$((${#MSG} + 1))"
  echo "}"
}  >> "${FILE}"

true > "${MANIFEST}"
{
  echo "SRC_FILE=${FILE}"
  echo "VERSION=${VERSION}"
  echo "DATE=${DATE}"
  echo "SEQUENCE_NUMBER=${SEQUENCE_NUMBER}"
} >> "${MANIFEST}"

