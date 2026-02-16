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
assert 9 '(1+2)*3'
assert 1 '(1)'
assert 47 '5+6*7'
assert 4 '(3+5)/2'
assert 10 '-10+20'
assert 10 '-(-10)'
assert 10 '- - - - (-10+20)*1'
assert 5 '- - - - (-10+20)/2'

assert 1 '0<1'
assert 0 '1<1'
assert 0 '2<1'
assert 1 '0<=1'
assert 1 '1<=1'
assert 0 '2<=1'

assert 1 '1>0'
assert 0 '1>1'
assert 0 '1>2'
assert 1 '1>=0'
assert 1 '1>=1'
assert 0 '1>=2'
echo OK