
# Syntax

## Syntax of the bytecode

A bytecode file records a series of instructions, possibly tagged by a
preceding label, separated by either newlines or semicolons. Spaces
except newline are insignificant.

	<code>  ::= <block> | <block> <newline> <code>
	<block> ::= <inst>
	          | <lab> : <inst>
	<newline> ∈ {'\n', ';'}

Each instruction takes one of the following forms.

	<inst> ::= <cmd/0> | <cmd/1> <arg>

where `<cmd/0>` is one of the following words,

	<cmd/0> ∈ { let  , endlet , add  , eq  , app
	          , tapp , mark   , grab , ret , halt }
	
and `<cmd/1>` takes one of the following forms,

	<cmd/1> ::= load <nat>
	          | load <bool>
			  | acc  <nat>
	          | b    <lab> { unconditional jump }
			  | bf   <lab> {     false-jump     }
			  | clos <lab> {      closure       }
			  
	<nat>  ∈ {0, 1, …}
	<bool> ∈ {true, false}
	<lab>  ∈ ℒ⁺

where `ℒ` denotes the set of characters excluding `:` and spaces.
Comments are followed by `#` and ended by a `<newline>`.
