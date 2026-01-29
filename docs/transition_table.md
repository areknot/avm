# Transition table

- `pc`: program counter
- `a`: accumulator
- `env`: environment
- `s`: argument stack
- `r`: return stack
- `e`: special symbol to separate arguments
- `b`: Boolean value
- `i`: Integer
- `n`: Natural number
  - `n⁺`: Positive natural number
- `l`: Label (address)

| Code[pc] |  Accu.   |      Env. | Arg. stack |  Ret. stack | Next pc |    Acc.    |              Env. | Arg. stack |    Ret. stack |
|:---------|:--------:|----------:|-----------:|------------:|:--------|:----------:|------------------:|-----------:|--------------:|
| push     |    a     |       env |          s |           r | pc++    |     a      |               env |       a::s |             r |
| load i   |    a     |       env |          s |           r | pc++    |     i      |               env |          s |             r |
| load b   |    a     |       env |          s |           r | pc++    |     b      |               env |          s |             r |
| acc  n   |    a     |       env |          s |           r | pc++    |   env[n]   |               env |          s |             r |
| clos l   |    a     |       env |          s |           r | pc++    |  <l,env>   |               env |          s |             r |
| let      |    v     |       env |          s |           r | pc++    |     v      |            v::env |          s |             r |
| endlet   |    a     |    v::env |          s |           r | pc++    |     a      |               env |          s |             r |
| b  l     |    a     |       env |          s |           r | l       |     a      |               env |          s |             r |
| bf l     |   true   |       env |          s |           r | pc++    |    true    |               env |          s |             r |
| bf l     |  false   |       env |          s |           r | l       |   false    |               env |          s |             r |
| add      |    n1    |       env |      n2::s |           r | pc++    |   n1+n2    |               env |          s |             r |
| eq       |    n1    |       env |      n2::s |           r | pc++    |   n1=n2    |               env |          s |             r |
| app      | <l,env'> |       env |       v::s |           r | l       |  <l,env'>  |           v::env' |          s | <pc++,env>::r |
| tapp     | <l,env'> |       env |       v::s |           r | l       |  <l,env'>  |           v::env' |          s |             r |
| mark     |    a     |       env |          s |           r | pc++    |     a      |               env |       e::s |             r |
| grab     |    a     |       env |       e::s | <l,env'>::r | l       | <pc++,env> |              env' |          s |             r |
| grab     |    a     |       env |       v::s |           r | pc++    |     a      |            v::env |          s |             r |
| ret      |    a     |       env |       e::s | <l,env'>::r | l       |     a      |              env' |          s |             r |
| ret      | <l,env'> |       env |       v::s |           r | l       |  <l,env'>  |           v::env' |          s |             r |
| dum n⁺   |    a     |       env |          s |           r | pc++    |     a      | <?,?>:…:<?,?>:env |          s |             r |
| upd n    | <l,env'> | <?,?>:env |          s |           r | pc++    |  <l,env'>  |    …<l,env'>…:env |          s |             r |

**Remark.** One may feel weird about the first transition rule of `ret` at the first glance. In fact, it works: the accumulator, which serves as a cache of the top element of the argument stack, represents the return-value. In the formulation without accumulator, we stored the return-value in the stack. When returning a value, we pop the top, see that the next element is ϵ, and push the return-value back to the stack. Using the new formulation, we just leave the accumulator untouched to achieve the same effect.

**Remark.** Now recursion is implemented by `dum` and `upd` (corresponding to *Dummy* and *Update* in Section 3.3.4, Leroy1990). Note that `dum m` prepends `m` dummy closures to the environment, and `upd n` updates the `n`-th dummy closure in the environment. The only difference with Leroy's implementation is that we restrict the dummy to be a closure. This is necessary as we cannot modify atomic data stored in environment (due to the copy of environment), and is reasonable for the current AVM. When we add compound data types, we may need to extend the transition table.

## Pseudo-compilation of ML subset to AVM

**Todo.** Update the compiler to support the accumulator.

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
