# declarative-turing-machine
Declarative Turing Machine specification and execution framework using C++26 annotation reflection.

Requires C++26 reflection support.
At the time of writing this requires Bloomberg's [P2996 experimental compiler](https://github.com/bloomberg/clang-p2996). 

Two example configurations are given: 'src/unary_to_binary.cpp', and 'src/flip_least_significant.cpp'.
### Flip Least Significant
A small example of a Turing Machine that flips the least significant bit of its input (e.g. '10110' -> '10100').

The configuration from this example is included here as a minimal syntax example:
```cpp
enum class Symbol {
	_,
	E,
	_0,
	_1
};

using enum Symbol;
using enum Action;

enum class [[=Config<Symbol>{"PrependEmpty", E, _}]] FlipLeastSignificant {
	PrependEmpty [[=RL<Symbol,
		{_0, E, Right, "ShiftRight_0"},
		{_1, E, Right, "ShiftRight_1"},
		{E, _, Halt, "PrependEmpty"}
	>]],
	ShiftRight_0 [[=RL<Symbol,
		{_0, _0, Right, "ShiftRight_0"},
		{_1, _0, Right, "ShiftRight_1"},
		{E, _0, Left, "ReverseScan"}
	>]],
	ShiftRight_1 [[=RL<Symbol,
		{_0, _1, Right, "ShiftRight_0"},
		{_1, _1, Right, "ShiftRight_1"},
		{E, _1, Left, "ReverseScan"}
	>]],
	ReverseScan [[=RL<Symbol,
		{_1, _0, Halt, "ReverseScan"},
		{E, _, Halt, "ReverseScan"},
		{_, _, Left, "ReverseScan"}
	>]],
};
```
### Unary to Binary
This implements a unary to binary conversion (e.g. '00000' -> '101') based on an old specification I wrote, with some changes to allow for halting behaviour.
The full specification is given here, there may be mistakes and it may be of variable quality, I haven't checked it thoroughly.
```
=============================================
; ALGORITHM

0. Prepend X and append YZ to string

1. Find first 0 or Y
    if Y [end]
    if 0 [continue]

2. Erase 0
3. Find next 0 or Y
    if Y [write 1; goto step 1]
    if 0 [continue]
4. Find next 0 or Y
    if Y [write 0; goto step 1]
    if 0 [goto step 2]

WRITE a:
0. Find Z
1. Write a
2. Write Z to next

END:
0. Copy contents between Y and Z to start in reverse order
1. Erase all proceeding symbols

=============================================
; TURING MACHINE

ST(s)                   ; Return to start of string
State   Symbol  Action  Next state
0       X       R       s
        [ELSE]  L       0


FF(s0, sy)              ; Find first 0 or Y
State   Symbol  Action  Next state
0       [ANY]   _       ST(1)
1       0       _       s0
        Y       _       sY
        [ELSE]  R       1


FN(s0, sy)              ; Find next 0 or Y
State   Symbol  Action  Next state
0       0       _       s0
        Y       _       sY
        [ELSE]  R       0


END(s)                  ; Find end of string (Z)
State   Symbol  Action  Next state
0       Z       _       s
        [ELSE]  R       0


W1(s)                   ; Write 1
State   Symbol  Action  Next state
0       [ANY]   _       END(1)
1       \Z      P1RPZ   s


W0(s)                   ; Write 0
State   Symbol  Action  Next state
0       [ANY]   _       END(1)
1       \Z      P0RPZ   s


CR0(s)                  ; Copy 0 and return
State   Symbol  Action  Next state
0       \0      PZ      1
1       C       P0RPC   END(s)
        X       P0RPC   END(s)
        [ELSE]  L       1


CR1(s)                  ; Copy 1 and return
State   Symbol  Action  Next state
0       \1      PZ      1
1       C       P1RPC   END(s)
        X       P1RPC   END(s)
        [ELSE]  L       1


FCR(s)                  ; Find C in reverse
State   Symbol  Action  Next state
0       C       _       s
        [ELSE]  L       0


TERM                    ; Terminate
State   Symbol  Action  Next state
0       [ANY]   _       END(1)
1       Y       _       FCR(2)
        0       _       CR0(1)
        1       _       CR1(1)
        Z       L       1
2       [ANY]   ER      2


SINK                    ; Do nothing forever
State   Symbol  Action  Next state
0       [ANY]   _       0


MAIN TABLE
State   Symbol  Action  Next state
0       0       PXR     1               ; Prelude
        _       P0      SINK
1       0       R       1
        _       P0R     2
2       [ANY]   PYRPZ   3               ; End prelude
3       [ANY]   _       FF(4, TERM)     ; Step 1
4       \0      E       FN(5, W1(3))    ; Step 2 and 3
5       \0      R       FN(4, W0(3))    ; Step 4

=============================================
; SIMULATED EXECUTION

INIT    .0000
MT:0    X.000
MT:1    X0.00
MT:1    X00.0
MT:1    X000.
MT:1    X0000.
MT:2    X0000Y.Z
MT:3    X0000Y.Z
    FF(4, TERM):0   X0000Y.Z
        ST(1):0     X0000.YZ
        ST(1):0     X000.0YZ
        ST(1):0     X00.00YZ
        ST(1):0     X0.000YZ
        ST(1):0     X.0000YZ
        ST(1):0     .X0000YZ
        ST(1):0     X.0000YZ
    FF(4, TERM):1   X.0000YZ
MT:4    X._000YZ
    FN(5, W1(3)):0  X_.000YZ
    FN(5, W1(3)):0  X_.000YZ
MT:5    X_0.00YZ
    FN(4, W0(3)):0  X_0.00YZ
MT:4    X_0._0YZ
    FN(5, W1(3))    ...     X_0_.0YZ
MT:5    X_0_0.YZ
    FN(4, W0(3))    ...     X_0_0.YZ
    W0(3):0     X_0_0.YZ
        END(1):0    X_0_0Y.Z
        END(1):0    X_0_0Y.Z
    W0(3):1     X_0_0Y0.Z
; Now skipping substeps
MT:3    X_.0_0Y0Z
MT:4    X___.0Y0Z
MT:5    X___0.Y0Z
    W0(3)   ...     X___0Y00.Z
MT:3    X___.0Y00Z
MT:4    X____.Y00Z
    W1(3)   ...     X____Y001.Z
MT:3    X____.Y001Z
    TERM:0  X____.Y001Z
        END(1)  ...     X____Y001.Z
    TERM:1  X____Y00.1Z
    TERM:1  X____Y00.1Z
        CR1(1)  ...     1C___Y00.ZZ
    TERM:1  1C___Y00.ZZ
    TERM:1  1C___Y0.0ZZ
    ; Now skipping substeps
    TERM:1  10C__Y0.ZZZ
    TERM:1  10C__Y.0ZZZ
    TERM:1  100C_Y.ZZZZ
    TERM:1  100C_.YZZZZ
    TERM:1  100.C_YZZZZ
    TERM:2  100_._YZZZZ
    TERM:2  100__.YZZZZ
    TERM:2  100___.ZZZZ
    TERM:2  100____.ZZZ
    TERM:2  100_____.ZZ
    TERM:2  100______.Z
    TERM:2  100_______.
    ...
```
