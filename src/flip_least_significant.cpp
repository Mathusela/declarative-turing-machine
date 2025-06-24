#include "decl_components.hpp"
#include "turing_machine.hpp"

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

int main() {
	TuringMachine<FlipLeastSignificant, Symbol> tm{};
	std::println("{}", tm.execute(std::vector{_1, _0, _1, _1, _0}) | std::views::transform(enum_to_string<Symbol>));
	
	tm.reset();
	std::println("{}", tm.execute(std::vector{_0, _1, _0, _1, _0}) | std::views::transform(enum_to_string<Symbol>));
	
	tm.reset();
	std::println("{}", tm.execute(std::vector{_0, _0, _0, _0, _0}) | std::views::transform(enum_to_string<Symbol>));
}