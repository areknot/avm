# Transition table

- `pc`: program counter
- `env`: environment
- `s`: argument stack
- `r`: return stack
- `e`: special symbol to separate arguments

| Code[pc] |   Env. |     Arg. stack |  Ret. stack | Next pc |               Env. |    Arg. stack |    Ret. stack |
|:---------|-------:|---------------:|------------:|:--------|-------------------:|--------------:|--------------:|
| load n   |    env |              s |           r | pc++    |                env |          n::s |             r |
| load b   |    env |              s |           r | pc++    |                env |          b::s |             r |
| acc i    |    env |              s |           r | pc++    |                env |     env[i]::s |             r |
| clos l   |    env |              s |           r | pc++    |                env |    <l,env>::s |             r |
| let      |    env |           v::s |           r | pc++    |             v::env |             s |             r |
| endlet   | v::env |              s |           r | pc++    |                env |             s |             r |
| b l      |    env |              s |           r | l       |                env |             s |             r |
| bf l     |    env |        true::s |           r | pc++    |                env |             s |             r |
| bf l     |    env |       false::s |           r | l       |                env |             s |             r |
| add      |    env |      n1::n2::s |           r | pc++    |                env |    (n1+n2)::s |             r |
| eq       |    env |      n1::n2::s |           r | pc++    |                env |   (n1==n2)::s |             r |
| app      |    env | <l,env'>::v::s |           r | l       |  v::<l,env'>::env' |             s | <pc++,env>::r |
| tapp     |    env | <l,env'>::v::s |           r | l       |  v::<l,env'>::env' |             s |             r |
| mark     |    env |              s |           r | pc++    |                env |          e::s |             r |
| grab     |    env |           e::s | <l,env'>::r | l       |               env' | <pc++,env>::s |             r |
| grab     |    env |           v::s |           r | pc++    | v::<pc++,env>::env |             s |             r |
| ret      |    env |        v::e::s | <l,env'>::r | l       |               env' |          v::s |             r |
| ret      |    env | <l,env'>::v::s |           r | l       |  v::<l,env'>::env' |             s |             r |


## Pseudo-compilation of ML subset to AVM

```
C(fun x -> e, venv) = Closure(l) ... l:T(e, x :: _ :: venv)
C(let rec f x = e1 in e2, venv) = Closure(l); Let; C(e2, f :: venv); EndLet ... l:T(e1, x :: f :: venv)
C(e e1 ... eN, venv) = PushMark; C(eN, venv); ...; C(e1, venv); C(e, venv); Apply
C(x) = Access(position x venv)
C(n, venv) = Ldi(n)
C(b, venv) = Ldb(b)
C(e1 + e2, venv) = C(e2, venv); C(e1, venv); Add
C(e1 = e2, venv) = C(e2, venv); C(e1, venv); Eq
C(let x = e1 in e2, venv) = C(e1, venv); Let; C(e2, x :: venv); EndLet
C(if e1 then e2 else e3, venv) = C(e1, venv); CJump(l1); C(e2, venv); ... l1:C(e3, venv)

T(fun x -> e, venv) = Grab; T(e, x :: _ :: venv)
T(let rec f x = e1 in e2, venv) = Closure(l); Let; T(e2, f :: venv) ... l:T(e1, x :: f :: venv)
T(e e1 ... eN, venv) = C(eN, venv); ...; C(e1, venv); C(e, venv); TailApply
T(x, venv) = Access(lookup venv x); Return
T(n, venv) = Ldi(n); Return
T(b, venv) = Ldb(b); Return
T(e1 + e2, venv) = C(e2, venv); C(e1, venv); Add; Return
T(e1 = e2, venv) = C(e2, venv); C(e1, venv); Eq; Return
T(let x = e1 in e2, venv) = C(e1, venv); Let; T(e2, x :: venv)
T(if e1 then e2 else e3, venv) = C(e1, venv); CJump(l1); T(e2, venv); ... l1:T(e3, venv)
```

Reference: https://www.logic.cs.tsukuba.ac.jp/jikken/zam.html
