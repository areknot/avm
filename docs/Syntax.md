
# Syntax

## Syntax of the bytecode

A bytecode file records a series of instructions, possibly tagged by a
preceding label. Spaces are insignificant (i.e., they may appear
anywhere).

	<code>  ::= <block> | <block> <code>
	<block> ::= <inst>  | <lab> : <inst>

Each instruction takes one of the following forms.

	<inst> ::= <cmd/0> | <cmd/1> <arg>

where `<cmd/0>` is one of the following words,

	<cmd/0> ∈ { let , endlet , add  , eq   , sub
	          , le  , app    , tapp , mark , grab
	          , ret , halt   , push   }
	
and `<cmd/1>` takes one of the following forms,

	<cmd/1> ::= load <int>
	          | load <bool>
	          | acc  <nat>
	          | b    <lab> {  unconditional jump   }
	          | bf   <lab> {    jump-if-false      }
	          | clos <lab> {       closure         }
			  | dum  <nat> {       dummies         }
			  | upd  <nat> { update the n-th dummy }
			  
	<nat>  ∈ {0, 1, …}
	<bool> ∈ {true, false}
	<int>  ∈ {…, -2, -1, 0, 1, …}
	<lab>  ∈ ℒ

where `ℒ` denotes the set of words (`[a-zA-Z_][a-zA-Z0-9_]*`). Spaces
cannot be put between sign (`-`) and numbers in integers.  Comments
follow `;` and are closed by a newline.

## Usage of the parser

The interfaces of the parser for AVM are in header `src/avm_parser.h`
and implement is in `src/avm_parser.c`. The parsing is accomplished by
a function

	AVM_code_t *parse(char *source, int size);

which takes the source code represented by a string (`source`) and its
lengths (`size`) and returns the result of parsing
(`AVM_code_t*`). Both the pointer `AVM_code_t*` and the `instr` field
inside it are allocated from heap.

The resulted pointer gets `NULL` if the parsing fails, due to syntax
error, backpatch error, or any other errors at runtime. The reason of
failure is detailed by a data structure `AVM_parse_error` in
`avm_parser.h`. The `message` field contains a text message describing
the error, and other fields are for locating the error. One can retrieve 
the error object by calling `AVM_parse_error* last_parse_error()` 
immediately after the failure of parsing.

## Example

Below is an example of the bytecode. The example is translated from
[here](https://www.cs.tsukuba.ac.jp/~kam/jikken/zam.html), with slight
modification.

```
; let rec sum x a =
;     if x = 0 then a
;     else sum (x - 1) (x + a)
; in sum 10 0
main:
    dum 1
    clos sum
    upd 0			; env = [sum]
    mark
    load 0  push
    load 10 push
    acc 0
    app
    endlet
    ret
sum:
    grab			; env = [a x sum]
    load 0 push
    acc 1
    eq
    bf L00
    acc 0
    ret
L00:
    acc 0  push
    acc 1
    add    push
    load 1 push
    acc 1
    sub    push
    acc 2
    tapp
```
