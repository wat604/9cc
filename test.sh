#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 10 "-10+20;"
assert 12 "-1*-(4-2)*6;"

assert 1 "2==2;"
assert 0 "2 == 44;"
assert 1 "4!=2;"
assert 0 "2!=2;"

assert 1 "10 > 8;"
assert 0 "8 > 10;"
assert 1 "2 >= 2;"
assert 0 "1 >= 10;"
assert 1 "15 < 120;"
assert 0 "8<4;"
assert 1 "4<43;"
assert 0 "23 <= 7;"
assert 1 "3 * 2 > 1 + 2 / 2;"

assert 4 "x = 3 + 1;"
assert 8 "x = y = (z = 2 * 2) * 2;"
# assert 1 "x - 2 = 1 + 1;"

echo OK