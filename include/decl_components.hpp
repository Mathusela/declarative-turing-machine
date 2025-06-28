#ifndef DECL_COMPONENTS_HPP
#define DECL_COMPONENTS_HPP

#include "utility.hpp"

#include <meta>
#include <utility>

enum class Action {
	Left,
	Right,
	None,
	Halt
};

struct StateRef {
	structural::StringView name;
	std::meta::info stateEnum;
};

template <typename Symbol>
struct Config {
	structural::StringView startStateName;
	Symbol emptySymbol;
	Symbol anySymbol;
};
namespace impl {
	template <typename Scope>
	consteval std::meta::info get_state_enum() {
		auto refl = ^^Scope;
		if (checked::is_enum_type(refl)) {
			if (!has_templated_annotation(refl, ^^Config))
				throw "Provided Enum scope is not correctly marked as a state Enum";
			return refl;
		}

		for (std::meta::info member : members_of(refl, std::meta::access_context::current())) {
			if (checked::is_enum_type(member) && has_templated_annotation(member, ^^Config))
				return member;
		}

		throw "Could not find valid state Enum";
	}

	template <typename Scope>
	consteval std::meta::info get_symbol() {
		auto stateEnum = get_state_enum<Scope>();
		for (std::meta::info a : annotations_of(stateEnum)) {
			std::meta::info type = type_of(a);
			if (has_template_arguments(type) && template_of(type) == ^^Config)
				return template_arguments_of(type)[0];
		}
		std::unreachable();
	}
}

// Required to allow nice type deduction rather than just having auto params
struct CallArg {
	std::meta::info refl;

	template <std::size_t n>
	consteval CallArg(const char(&str)[n])
		: refl(std::meta::reflect_constant(structural::StringView(str))) {}

	consteval CallArg(structural::StringView sv)
		: refl(std::meta::reflect_constant(sv)) {}
	
	consteval CallArg(StateRef sr)
		: refl(std::meta::reflect_constant(sr)) {}
	
	// Auto to break circular dependency
	consteval CallArg(auto call)
		: refl(std::meta::reflect_constant(call)) {}
};

template <template <auto...> typename Machine, CallArg... args>
struct CallLinker {};
template <typename Machine>
struct ConcreteCallLinker {};

template <template <auto...> typename Machine, CallArg... args>
inline CallLinker<Machine, args...> Call;
template <typename Machine>
inline ConcreteCallLinker<Machine> CCall;

namespace impl {
	template <std::meta::info stateEnum>
	consteval auto get_start_state_name() {
		for (std::meta::info a : annotations_of(stateEnum)) {
			std::meta::info type = type_of(a);
			// Compiler was complaining so we have to do this in a really roundabout way?
			if (has_template_arguments(type) && template_of(type) == ^^Config)
				return extract<typename [:substitute(
					^^Config,
					{get_symbol<typename [:stateEnum:]>()}
				):]>(a).startStateName;
		}
		std::unreachable();
	}

	template <typename T>
	constexpr auto& prepend_helper(T x, std::vector<T>& vec) {
		vec.insert(vec.begin(), x);
		return vec;
	}
	template <typename T>
	constexpr auto prepend_helper(T x, std::vector<T>&& vec) {
		vec.insert(vec.begin(), x);
		return vec;
	}

	template <typename Machine>
	consteval StateRef resolve_concrete_call();

	template <typename CurrentStateEnum, template <auto...> typename Machine, CallArg... args>
	consteval StateRef resolve_parameterized_call() {
		constexpr auto refs = [] {
			std::vector<std::meta::info> refs{};
			
			auto handle_arg = [&refs]<CallArg arg>() {
				constexpr auto type = decay(type_of(arg.refl));
				if constexpr (type == ^^StateRef)
					refs.push_back(arg.refl);
				else if constexpr (type == ^^structural::StringView)
					refs.push_back(std::meta::reflect_constant(StateRef{[:arg.refl:], ^^CurrentStateEnum}));
				else if constexpr (has_template_arguments(type) && template_of(type) == ^^ConcreteCallLinker) {
					refs.push_back(std::meta::reflect_constant(
						resolve_concrete_call<typename [:template_arguments_of(type)[0]:]>()
					));
				}
				else if constexpr (has_template_arguments(type) && template_of(type) == ^^CallLinker) {
					refs.push_back(std::meta::reflect_constant(
						[:substitute(^^resolve_parameterized_call, prepend_helper(^^CurrentStateEnum, template_arguments_of(type))):]()
					)); 
				}
				else
					throw "Unexpected type in Call parameter";
			};
			(handle_arg.template operator()<args>(), ...);

			return std::define_static_array(refs);
		}();
		
		constexpr auto stateEnum = impl::get_state_enum<typename [:substitute(^^Machine, refs):]>();
		return StateRef{get_start_state_name<stateEnum>(), stateEnum};
	}

	template <typename Machine>
	consteval StateRef resolve_concrete_call() {
		constexpr auto stateEnum = impl::get_state_enum<Machine>();
		return StateRef{get_start_state_name<stateEnum>(), stateEnum};
	}
};

template <typename Scope>
struct Response {
	using Symbol = [:impl::get_symbol<Scope>():];

	Symbol read;
	Symbol write;
	Action action;
	StateRef nextState;

	constexpr Response(Symbol read, Symbol write, Action action, structural::StringView nextStateName)
		: read(read), write(write), action(action), nextState(nextStateName, impl::get_state_enum<Scope>()) {}

	constexpr Response(Symbol read, Symbol write, Action action, StateRef nextState)
		: read(read), write(write), action(action), nextState(nextState) {}

	template <template <auto...> typename Machine, CallArg... args>
	constexpr Response(Symbol read, Symbol write, Action action, CallLinker<Machine, args...> call)
		: read(read), write(write), action(action), nextState(impl::resolve_parameterized_call<typename [:impl::get_state_enum<Scope>():], Machine, args...>()) {}
	
	template <typename Machine>
	constexpr Response(Symbol read, Symbol write, Action action, ConcreteCallLinker<Machine> call)
		: read(read), write(write), action(action), nextState(impl::resolve_concrete_call<Machine>()) {}
};

template <typename Scope, Response<Scope>...>
struct ResponseList {};

template <typename Scope, Response<Scope>... Rs>
inline ResponseList<Scope, Rs...> RL;

#endif // DECL_COMPONENTS_HPP