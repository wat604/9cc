#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s test_func.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

run_func() {
    input="$1"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s test_func.o
    ./tmp
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

assert 1 "foo = 1;"
assert 5 "bar = 2 + 3;"
assert 3 "foo = 1; bar = 2; foo + bar;"
assert 1 "_A1z = 1;"

assert 1 "return 1; 2; 3;"
assert 2 "1; return 2; 3;"
assert 3 "1; 2; return 3;"

assert 2 "x = 1; y = 0; if (x == 1) y = 2; return y;"
assert 0 "x = 1; y = 0; if (x == 0) y = 2; return y;"
assert 2 "x = 1; if (x == 1) y = 2; else y = 3; return y;"
assert 3 "x = 1; if (x == 0) y = 2; else y = 3; return y;"
assert 0 "x = 1; y = 2; z = 3; if (x == 1) if (y == 2) z = 0; return z;"
assert 3 "x = 1; y = 2; z = 3; if (x == 1) if (y == 0) z = 0; return z;"

assert 10 "i = 0; while (i < 10) i = i + 1; return i;"

assert 20 "x = 0; for (i = 0; i < 10; i = i + 1) x = x + 2; return x;"
assert 20 "x = 0; i = 0; for (; i < 10; i = i + 1) x = x + 2; return x;"
assert 10 "for (i = 0; ; i = i + 1) return 10;"
assert 10 "for (i = 0; i < 10; ) i = i + 1; return i;"

assert 3 "{ x = 1; y = 2; z = x + y; } return z;"
assert 2 "if (1) {y = 1; y = y + 1;} return y;"

run_func "foo();"
assert 3 "return myadd(1, 2);"
assert 6 "return myadd(1, 2 + 3);"
assert 6 "return myadd(1, myadd(2, 3));"
assert 10 "return myadd(1, myadd(myadd(2, 4), 3));"


echo OK