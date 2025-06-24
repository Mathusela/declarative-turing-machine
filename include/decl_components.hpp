#ifndef DECL_COMPONENTS_HPP
#define DECL_COMPONENTS_HPP

#include "utility.hpp"

enum class Action {
	Left,
	Right,
	None,
	Halt
};

template <typename Symbol>
struct Response {
	Symbol read;
	Symbol write;
	Action action;
	structural::StringView nextStateName;
};

template <typename Symbol, Response<Symbol>...>
struct ResponseList {};

template <typename Symbol, Response<Symbol>... Rs>
inline ResponseList<Symbol, Rs...> RL;

template <typename Symbol>
struct Config {
	structural::StringView startStateName;
	Symbol emptySymbol;
	Symbol anySymbol;
};

#endif // DECL_COMPONENTS_HPP