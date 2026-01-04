# Notes on performance

In this document, we compare the performance of "stacks as linked lists" and "stacks as arrays".

The following code, immitating the Tarai function, is used as a benchmark.

```
main:
    clos L_tarai
    let
    mark
    load 0
    load 7
    load 13
    acc 0
    app
    endlet
    ret
L_tarai:
    grab
    grab
    acc 4
    acc 2
    le
    bf L_else
    acc 2
    ret
L_else:
    mark
    acc 2
    acc 4
    acc 0
    load 1
    sub
    acc 5
    app
    mark
    acc 4
    acc 0
    acc 2
    load 1
    sub
    acc 5
    app
    mark
    acc 0
    acc 2
    acc 4
    load 1
    sub
    acc 5
    app
    acc 5
    tapp
```


- `Tarai(x, y, z) = y` if `x <= y`
- `Tarai(x, y, z) = Tarai(Tarai(x-1, y z), Tarai(y-1, z, x), Tarai(z-1, x, y))`

## Stack as linked lists

When stacks are implemented using linked lists, executing the code above took 43.02 seconds.

## Stack as dynamic arrays

When changing the implementation of stacks from linked lists to dynamic arrays, the execusion of the code above took 24.03 seconds.

## Other languages 

For other mainstream languages, C took 0.13 seconds, COBOL took 29.43 seconds, OCaml took 1.10 seconds, Python took 4.34 seconds, and Ruby took 2.11 seconds.

C: `gcc tarai.c -o tarai && ./tarai`

``` c
#include <stdio.h>

int tarai(int x, int y, int z) {
  if (x <= y) {
    return y;
  } else {
    return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y));
  }
}

int main(void) {
  printf("%d\n", tarai(13, 7, 0));

  return 0;
}
```

COBOL: `cobc -o a.out tarai.cbl && ./a.out`

``` cobol
IDENTIFICATION DIVISION.
FUNCTION-ID. TARAI.
DATA DIVISION.
LOCAL-STORAGE SECTION.
01 x1 PIC S9(8).
01 y1 PIC S9(8).
01 z1 PIC S9(8).
LINKAGE SECTION.
01 x0 PIC S9(8).
01 y0 PIC S9(8).
01 z0 PIC S9(8).
01 r  PIC S9(8).
PROCEDURE DIVISION USING x0 y0 z0 RETURNING r.
IF x0 <= y0 THEN
    COMPUTE r = y0
ELSE
    COMPUTE x1 = x0 - 1
    COMPUTE y1 = y0 - 1
    COMPUTE z1 = z0 - 1
    COMPUTE r =
        FUNCTION TARAI (
            FUNCTION TARAI (x1 y0 z0)
            FUNCTION TARAI (y1 z0 x0)
            FUNCTION TARAI (z1 x0 y0)
        )
END-IF.
EXIT FUNCTION.
END FUNCTION TARAI.
FICATION DIVISION.
PROGRAM-ID. MAIN.
ENVIRONMENT DIVISION.
CONFIGURATION SECTION.
REPOSITORY.
    FUNCTION TARAI.
DATA DIVISION.
WORKING-STORAGE SECTION.
01 xn PIC S9(8).
01 yn PIC S9(8).
01 zn PIC S9(8).
PROCEDURE DIVISION.
    COMPUTE xn = 13.
    COMPUTE yn = 7.
    COMPUTE zn = 0.
    DISPLAY FUNCTION TARAI (xn yn zn).
    STOP RUN.
```

OCaml: `ocaml tarai.ml`

``` ocaml
let rec tarai x y z =
  if x <= y then y
  else tarai (tarai (x - 1) y z) (tarai (y - 1) z x) (tarai (z - 1) x y)

let () = Printf.printf "%d\n" (tarai 13 7 0)
```

Python: `python3 tarai.py`

``` python
def tarai(x: int, y: int, z: int) -> int:
    if x <= y:
        return y
    else:
        return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y))

print(tarai(13, 7, 0))
```

Ruby: `ruby tarai.rb`

``` ruby
def tarai(x, y, z)
  if x <= y
    return y
  else
    return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y))
  end
end

puts(tarai(13, 7, 0))
```

These code samples were took from https://qiita.com/Freezer/items/1badfbba34c13995eed4#c and executed on a Mac mini with Apple M4 Pro CPU and 24 GB memory.
