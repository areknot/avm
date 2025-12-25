# Transition table

`pc`: program counter
`env`: environment
`s`: argument stack
`r`: return stack
`e`: special symbol to separate arguments

| Code[pc]   | Env.   | Arg. stack     | Ret. stack  | Next pc | Env.               | Arg. stack    | Ret. stack    |
|:-----------|:-------|:---------------|:------------|:--------|:-------------------|:--------------|:--------------|
| Ldi(n)     | env    | s              | r           | pc++    | env                | n::s          | r             |
| Ldb(b)     | env    | s              | r           | pc++    | env                | b::s          | r             |
| Access(i)  | env    | s              | r           | pc++    | env                | env(i)::s     | r             |
| Closure(l) | env    | s              | r           | pc++    | env                | <l,env>::s    | r             |
| Let        | env    | v::s           | r           | pc++    | v::env             | s             | r             |
| EndLet     | v::env | s              | r           | pc++    | env                | s             | r             |
| Jump(l)    | env    | s              | r           | l       | env                | s             | r             |
| CJump(l)   | env    | true::s        | r           | pc++    | env                | s             | r             |
| CJump(l)   | env    | false::s       | r           | l       | env                | s             | r             |
| Add        | env    | n1::n2::s      | r           | pc++    | env                | (n1+n2)::s    | r             |
| Eq         | env    | n1::n2::s      | r           | pc++    | env                | (n1==n2)::s   | r             |
| Apply      | env    | <l,env'>::v::s | r           | l       | v::<l,env'>::env'  | s             | <pc++,env>::r |
| TailApply  | env    | <l,env'>::v::s | r           | l       | v::<l,env'>::env'  | s             | r             |
| PushMark   | env    | s              | r           | pc++    | env                | e::s          | r             |
| Grab       | env    | e::s           | <l,env'>::r | l       | env'               | <pc++,env>::s | r             |
| Grab       | env    | v::s           | r           | pc++    | v::<pc++,env>::env | s             | r             |
| Return     | env    | v::e::s        | <l,env'>::r | l       | env'               | v::s          | r             |
| Return     | env    | <l,env'>::v::s | r           | l       | v::<l,env'>::env'  | s             | r             |
