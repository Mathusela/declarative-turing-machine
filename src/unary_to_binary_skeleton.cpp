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

// Return to start of string
template <StateRef s>
struct St {
	enum [[=Config<Symbol>{"Start", E, _}]] State {
		Start [[=RL<State,
			{X, _, Right, s},
			{_, _, Left, "Start"}
		>]]
	};
};

// Find first 0 or Y
template <StateRef s0, StateRef sY>
struct Ff {
	enum [[=Config<Symbol>{"ReturnToStart", E, _}]] State {
		ReturnToStart [[=RL<State,
			{_, _, None, Call<St, "ScanRight">}
		>]],
		ScanRight [[=RL<State,
			{_0, _, None, s0},
			{Y, _, None, sY},
			{_, _, Right, "ScanRight"}
		>]]
	};
};

// Find next 0 or Y
template <StateRef s0, StateRef sY>
struct Fn {
	enum [[=Config<Symbol>{"ScanRight", E, _}]] State {
		ScanRight [[=RL<State,
			{_0, _, None, s0},
			{Y, _, None, sY},
			{_, _, Right, "ScanRight"}
		>]]
	};
};

// Find end of string (Z)
template <StateRef s>
struct End {
	enum [[=Config<Symbol>{"ScanRight", E, _}]] State {
		ScanRight [[=RL<State,
			{Z, _, None, s},
			{_, _, Right, "ScanRight"}
		>]]
	};
};

// Write 1
template <StateRef s>
struct W1 {
	enum [[=Config<Symbol>{"FindEnd", E, _}]] State {
		FindEnd [[=RL<State,
			{_, _, None, Call<End, "Write1">}
		>]],
		Write1 [[=RL<State,
			{Z, _1, Right, "WriteZ"}
		>]],
		WriteZ [[=RL<State,
			{_, Z, None, s}
		>]]
	};
};

// Write 0
template <StateRef s>
struct W0 {
	enum [[=Config<Symbol>{"FindEnd", E, _}]] State {
		FindEnd [[=RL<State,
			{_, _, None, Call<End, "Write0">}
		>]],
		Write0 [[=RL<State,
			{Z, _0, Right, "WriteZ"}
		>]],
		WriteZ [[=RL<State,
			{_, Z, None, s}
		>]]
	};
};

// Copy 0 and return
template <StateRef s>
struct Cr0 {
	enum [[=Config<Symbol>{"WriteZ", E, _}]] State {
		WriteZ [[=RL<State,
			{_0, Z, None, "Copy"}
		>]],
		Copy [[=RL<State,
			{C, _0, Right, "WriteC"},
			{X, _0, Right, "WriteC"},
			{_, _, Left, "Copy"}
		>]],
		WriteC [[=RL<State,
			{_, C, None, Call<End, s>}
		>]]
	};
};

// Copy 1 and return
template <StateRef s>
struct Cr1 {
	enum [[=Config<Symbol>{"WriteZ", E, _}]] State {
		WriteZ [[=RL<State,
			{_1, Z, None, "Copy"}
		>]],
		Copy [[=RL<State,
			{C, _1, Right, "WriteC"},
			{X, _1, Right, "WriteC"},
			{_, _, Left, "Copy"}
		>]],
		WriteC [[=RL<State,
			{_, C, None, Call<End, s>}
		>]]
	};
};

// Find C in reverse
template <StateRef s>
struct Fcr {
	enum [[=Config<Symbol>{"ScanLeft", E, _}]] State {
		ScanLeft [[=RL<State,
			{C, _, None, s},
			{_, _, Left, "ScanLeft"}
		>]]
	};
};

// Terminate
enum class [[=Config<Symbol>{"FindEnd", E, _}]] Term {
	FindEnd [[=RL<Term,
		{_, _, None, Call<End, "Copy">}
	>]],
	Copy [[=RL<Term,
		{Y, _, None, Call<Fcr, "Overwrite">},
		{_0, _, None, Call<Cr0, "Copy">},
		{_1, _, None, Call<Cr1, "Copy">},
		{Z, _, Left, "Copy"}
	>]],
	Overwrite [[=RL<Term,
		{Z, E, Right, "End"},
		{_, E, Right, "Overwrite"}
	>]],
	End [[=RL<Term,
		{E, _, Halt, "End"},
		{_, E, Right, "End"}
	>]]
};

// Halt [Do nothing forever]
enum class [[=Config<Symbol>{"End", E, _}]] Sink {
	End [[=RL<Sink,
		{_, _, Halt, "End"}
	>]]
};

enum class [[=Config<Symbol>{"Prelude0", E, _}]] Main {
	// Prelude
	Prelude0 [[=RL<Main,
		{_0, X, Right, "Prelude1"},
		{E, _0, None, CCall<Sink>}
	>]],
	Prelude1 [[=RL<Main,
		{_0, _, Right, "Prelude1"},
		{E, _0, Right, "Prelude2_0"}
	>]],
	Prelude2_0 [[=RL<Main,
		{_, Y, Right, "Prelude2_1"}
	>]],
	Prelude2_1 [[=RL<Main,
		{_, Z, None, "Step1"}
	>]],
	// Step 1
	Step1 [[=RL<Main,
		{_, _, None, Call<Ff, "Step23", CCall<Term>>}
	>]],
	// Step 2 and 3
	Step23 [[=RL<Main,
		{_0, E, None, Call<Fn, "Step4", Call<W1, "Step1">>}
	>]],
	// Step 4
	Step4 [[=RL<Main,
		{_0, _, Right, Call<Fn, "Step23", Call<W0, "Step1">>}
	>]]
};

int main() {
	TuringMachine<Main> tm{};
	std::println("{}", tm.execute({_0, _0, _0, _0, _0}) | std::views::transform(state_variant_to_string));
}