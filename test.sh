#!/bin/bash

CC=${1:-./Chibicc}

assert() {
  expected="$1"
  input="$2"

 
  "$CC" "$input" > tmp.s || exit
  
  gcc -static -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42
assert 7 '17-10+0'
assert 10 ' 17 - 7+0'

echo OK