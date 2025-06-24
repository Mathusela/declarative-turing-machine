#include "decl_components.hpp"
#include "turing_machine.hpp"

enum class Symbol {
	E,
	_,
	_0,
	_1,
	X,
	Y,
	Z,
	C
};

using enum Symbol;
using enum Action;

enum class [[=Config<Symbol>{"Main_0", E, _}]] State {
	// Prepend X and append YZ to string
	Main_0 [[=RL<Symbol,
		{_0, X, Right, "Main_1"},
		{E, _0, None, "Sink_0"}
	>]],
	Main_1 [[=RL<Symbol,
		{_0, _, Right, "Main_1"},
		{E, _0, Right, "Main_2_0"}
	>]],
	Main_2_0 [[=RL<Symbol,
		{_, Y, Right, "Main_2_1"}
	>]],
	Main_2_1 [[=RL<Symbol,
		{_, Z, None, "Main_3"}
	>]],
	Main_3 [[=RL<Symbol,
		{_, _, None, "Ff$Main_4$Term_0$_0"}
	>]],
	Main_4 [[=RL<Symbol,
		{_0, E, None, "Fn$Main_5$W1$$Main_3$$_0$_0"}
	>]],
	Main_5 [[=RL<Symbol,
		{_0, _, Right, "Fn$Main_4$W0$$Main_3$$_0$_0"}
	>]],

	// ==========
	// ST(s)
	// Return to start of string
	// St_0 [[=RL<Symbol,
	// 	{X, _, Right, /*s*/},
	// 	{_, _, Left, /*St_0*/}
	// >]],

	// ST(Ff$Main_4$Term_0$_1)
	St$Ff$$Main_4$$Term_0$$_1$_0 [[=RL<Symbol,
		{X, _, Right, "Ff$Main_4$Term_0$_1"},
		{_, _, Left, "St$Ff$$Main_4$$Term_0$$_1$_0"}
	>]],
	// ==========

	// ==========
	// FF(s0, sy)
	// Find first 0 or Y
	// Ff_0 [[=RL<Symbol,
	// 	{_, _, None, /*St(Ff_1)*/}
	// >]],
	// Ff_1 [[=RL<Symbol,
	// 	{_0, _, None, /*s0*/},
	// 	{Y, _, None, /*sY*/},
	// 	{_, _, Right, /*Ff_1*/}
	// >]],

	// FF(Main_4, Term_0)
	Ff$Main_4$Term_0$_0 [[=RL<Symbol,
		{_, _, None, "St$Ff$$Main_4$$Term_0$$_1$_0"}
	>]],
	Ff$Main_4$Term_0$_1 [[=RL<Symbol,
		{_0, _, None, "Main_4"},
		{Y, _, None, "Term_0"},
		{_, _, Right, "Ff$Main_4$Term_0$_1"}
	>]],
	// ==========

	// ==========
	// FN(s0, sy)
	// Find next 0 or Y
	// Fn_0 [[=RL<Symbol,
	// 	{_0, _, None, /*s0*/},
	// 	{Y, _, None, /*sY*/},
	// 	{_, _, Right, /*Fn_0*/}
	// >]],

	// FN(Main_5, W1(Main_3))
	Fn$Main_5$W1$$Main_3$$_0$_0 [[=RL<Symbol,
		{_0, _, None, "Main_5"},
		{Y, _, None, "W1$Main_3$_0"},
		{_, _, Right, "Fn$Main_5$W1$$Main_3$$_0$_0"}
	>]],

	// Fn(Main_4, W0(Main_3))
	Fn$Main_4$W0$$Main_3$$_0$_0 [[=RL<Symbol,
		{_0, _, None, "Main_4"},
		{Y, _, None, "W0$Main_3$_0"},
		{_, _, Right, "Fn$Main_4$W0$$Main_3$$_0$_0"}
	>]],
	// ==========

	// ==========
	// END(s)
	// Find end of string (Z)
	// End_0 [[=RL<Symbol,
	// 	{Z, _, None, /*s*/},
	// 	{_, _, Right, /*End_0*/}
	// >]],

	// END(W1$Main_3$_1_0)
	End$W1$$Main_3$$_1_0$_0 [[=RL<Symbol,
		{Z, _, None, "W1$Main_3$_1_0"},
		{_, _, Right, "End$W1$$Main_3$$_1_0$_0"}
	>]],

	// END(W0$Main_3$_1_0)
	End$W0$$Main_3$$_1_0$_0 [[=RL<Symbol,
		{Z, _, None, "W0$Main_3$_1_0"},
		{_, _, Right, "End$W0$$Main_3$$_1_0$_0"}
	>]],

	// END(Term_1)
	End$Term_1$_0 [[=RL<Symbol,
		{Z, _, None, "Term_1"},
		{_, _, Right, "End$Term_1$_0"}
	>]],
	// ==========

	// ==========
	// W1(s)
	// Write 1
	// W1_0 [[=RL<Symbol,
	// 	{_, _, None, /*End(W1_1_0)*/}
	// >]],
	// W1_1_0 [[=RL<Symbol,
	// 	{Z, _1, Right, /*W1_1_1*/}
	// >]],
	// W1_1_1 [[=RL<Symbol,
	// 	{_, Z, None, /*s*/}
	// >]],

	// W1(Main_3)
	W1$Main_3$_0 [[=RL<Symbol,
		{_, _, None, "End$W1$$Main_3$$_1_0$_0"}
	>]],
	W1$Main_3$_1_0 [[=RL<Symbol,
		{Z, _1, Right, "W1$Main_3$_1_1"}
	>]],
	W1$Main_3$_1_1 [[=RL<Symbol,
		{_, Z, None, "Main_3"}
	>]],
	// ==========

	// ==========
	// W0(s)
	// Write 0
	// W0_0 [[=RL<Symbol,
	// 	{_, _, None, /*End(W0_1_0)*/}
	// >]],
	// W0_1_0 [[=RL<Symbol,
	// 	{Z, _0, Right, /*W0_1_1*/}
	// >]],
	// W0_1_1 [[=RL<Symbol,
	// 	{_, Z, None, /*s*/}
	// >]],

	// W0(Main_3)
	W0$Main_3$_0 [[=RL<Symbol,
		{_, _, None, "End$W0$$Main_3$$_1_0$_0"}
	>]],
	W0$Main_3$_1_0 [[=RL<Symbol,
		{Z, _0, Right, "W0$Main_3$_1_1"}
	>]],
	W0$Main_3$_1_1 [[=RL<Symbol,
		{_, Z, None, "Main_3"}
	>]],
	// ==========

	// ==========
	// CR0(s)
	// Copy 0 and return
	// Cr0_0 [[=RL<Symbol,
	// 	{_0, Z, None, /*Cr0_1_0*/}
	// >]],
	// Cr0_1_0 [[=RL<Symbol,
	// 	{C, _0, Right, /*Cr0_1_1*/},
	// 	{X, _0, Right, /*Cr0_1_1*/},
	// 	{_, _, Left, /*Cr0_1_0*/}
	// >]],
	// Cr0_1_1 [[=RL<Symbol,
	// 	{_, C, None, /*End(s)*/}
	// >]],

	// CR0(Term_1)
	Cr0$Term_1$_0 [[=RL<Symbol,
		{_0, Z, None, "Cr0$Term_1$_1_0"}
	>]],
	Cr0$Term_1$_1_0 [[=RL<Symbol,
		{C, _0, Right, "Cr0$Term_1$_1_1"},
		{X, _0, Right, "Cr0$Term_1$_1_1"},
		{_, _, Left, "Cr0$Term_1$_1_0"}
	>]],
	Cr0$Term_1$_1_1 [[=RL<Symbol,
		{_, C, None, "End$Term_1$_0"}
	>]],
	// ==========

	// ==========
	// CR1(s)
	// Copy 1 and return
	// Cr1_0 [[=RL<Symbol,
	// 	{_1, Z, None, /*Cr1_1_0*/}
	// >]],
	// Cr1_1_0 [[=RL<Symbol,
	// 	{C, _1, Right, /*Cr1_1_1*/},
	// 	{X, _1, Right, /*Cr1_1_1*/},
	// 	{_, _, Left, /*Cr1_1_0*/}
	// >]],
	// Cr1_1_1 [[=RL<Symbol,
	// 	{_, C, None, /*End(s)*/}
	// >]],

	// CR1(Term_1)
	Cr1$Term_1$_0 [[=RL<Symbol,
		{_1, Z, None, "Cr1$Term_1$_1_0"}
	>]],
	Cr1$Term_1$_1_0 [[=RL<Symbol,
		{C, _1, Right, "Cr1$Term_1$_1_1"},
		{X, _1, Right, "Cr1$Term_1$_1_1"},
		{_, _, Left, "Cr1$Term_1$_1_0"}
	>]],
	Cr1$Term_1$_1_1 [[=RL<Symbol,
		{_, C, None, "End$Term_1$_0"}
	>]],
	// ==========

	// ==========
	// FCR(s)
	// Find C in reverse
	// Fcr_0 [[=RL<Symbol,
	// 	{C, _, None, /*s*/},
	// 	{_, _, Left, /*Fcr_0*/}
	// >]],

	// FCR(Term_2)
	Fcr$Term_2$_0 [[=RL<Symbol,
		{C, _, None, "Term_2"},
		{_, _, Left, "Fcr$Term_2$_0"}
	>]],
	// ==========

	// TERM
	// Terminate
	Term_0 [[=RL<Symbol,
		{_, _, None, "End$Term_1$_0"}
	>]],
	Term_1 [[=RL<Symbol,
		{Y, _, None, "Fcr$Term_2$_0"},
		{_0, _, None, "Cr0$Term_1$_0"},
		{_1, _, None, "Cr1$Term_1$_0"},
		{Z, _, Left, "Term_1"}
	>]],
	Term_2 [[=RL<Symbol,
		{Z, E, Right, "Term_3"},
		{_, E, Right, "Term_2"}
	>]],
	Term_3 [[=RL<Symbol,
		{E, _, Halt, "Term_3"},
		{_, E, Right, "Term_3"}
	>]],

	// SINK
	// Halt [Do nothing forever]
	Sink_0 [[=RL<Symbol,
		{_, _, Halt, "Sink_0"}
	>]], 
};

int main() {
	TuringMachine<State, Symbol> tm{};
	std::vector input = {_0, _0, _0, _0, _0};
	std::println("{}", tm.execute(input) | std::views::transform(enum_to_string<Symbol>));
}