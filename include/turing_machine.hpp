#ifndef TURING_MACHINE_HPP
#define TURING_MACHINE_HPP

#include "utility.hpp"
#include "decl_components.hpp"

#include <vector>
#include <ranges>
#include <meta>
#include <stdexcept>
#include <span>
#include <optional>
#include <iterator>
#include <utility>
#include <print>
#include <set>
#include <concepts>
#include <variant>

namespace impl {
	template <typename T>
	struct LinearSet {
		std::vector<T> data;
		
		constexpr void insert(T x) {
			if (!std::ranges::contains(data, x))
				data.push_back(std::move(x));
		}
	};

	struct ReachableStatesAndSymbols {
		LinearSet<std::meta::info> states;
		LinearSet<std::meta::info> symbols;
	};


	template <std::meta::info e>
	consteval auto get_enumerator_linked_scopes() -> LinearSet<std::meta::info> {
		LinearSet<std::meta::info> linkedScopes;
		template for (constexpr auto a : std::define_static_array(annotations_of(e))) {
			constexpr auto type = type_of(a);
			if constexpr (has_template_arguments(type) && template_of(type) == ^^ResponseList) {
				// Found response list
				template for (constexpr auto responseInfo : std::define_static_array(template_arguments_of(type) | std::views::drop(1))) {
					constexpr auto response = [:responseInfo:];
					linkedScopes.insert(response.nextState.stateEnum);
				}
			}
		}
		return linkedScopes;
	}

	template <typename Descriptor>
	consteval void get_reachable_states_and_symbols_helper(ReachableStatesAndSymbols& out) {
		out.symbols.insert(impl::get_symbol<Descriptor>());
		constexpr auto stateEnum = impl::get_state_enum<Descriptor>();
		out.states.insert(stateEnum);
		
		template for (constexpr auto e : std::define_static_array(enumerators_of(stateEnum))) {
			template for (constexpr auto scope : std::define_static_array(get_enumerator_linked_scopes<e>().data)) {
				if (!std::ranges::contains(out.states.data, scope))
					get_reachable_states_and_symbols_helper<typename [:scope:]>(out);
			}
		}
	}

	// TODO: Assert that empty symbols match. --actually, maybe just allow dropping empty symbols from non-entry machines, since emtpy symbols can never match with different alphabets
	template <typename Descriptor>
	consteval ReachableStatesAndSymbols get_reachable_states_and_symbols() {
		ReachableStatesAndSymbols out;
		get_reachable_states_and_symbols_helper<Descriptor>(out);
		return out;
	}

	template <typename Descriptor>
	struct ComputedVariants {
		using State = [:substitute(^^std::variant, get_reachable_states_and_symbols<Descriptor>().states.data):];
		using Symbol = [:substitute(^^std::variant, get_reachable_states_and_symbols<Descriptor>().symbols.data):];
	};
};

template <typename Descriptor>
class TuringMachine {
	using StateVariant = impl::ComputedVariants<Descriptor>::State;
	using SymbolVariant = impl::ComputedVariants<Descriptor>::Symbol;

	std::vector<SymbolVariant> tape_;
	std::ranges::iterator_t<decltype(tape_)> head_;
	StateVariant state_;
	SymbolVariant emptySymbol_;

	template <typename Symbol>
	struct ResponseVariant {
		Symbol read;
		Symbol write;
		Action action;
		structural::StringView nextStateName;
		StateVariant nextStateEnum;

		template <auto response>
		static constexpr ResponseVariant<Symbol> create() {
			return ResponseVariant<Symbol>{
				.read = response.read,
				.write = response.write,
				.action = response.action,
				.nextStateName = response.nextState.name,
				.nextStateEnum = typename [:response.nextState.stateEnum:]{}
			};
		}
	};

	template <std::meta::info responseList>
	[[nodiscard]] static constexpr auto get_response_from_list(auto head, auto anySymbol) {
		template for (constexpr std::meta::info responseInfo : std::define_static_array(template_arguments_of(responseList) | std::views::drop(1))) {
			constexpr auto response = [:responseInfo:];
			bool match = response.read == std::get<typename decltype(response)::Symbol>(*head);
			if (match || response.read == anySymbol)
				return ResponseVariant<typename decltype(response)::Symbol>::template create<response>();
				// return ScopedResponse<typename [:response.nextState.stateEnum:], typename decltype(response)::Symbol>{response};
		}
		throw std::runtime_error("Could not find valid Response");
	}

	template <typename State>
	[[nodiscard]] static constexpr auto get_response(State state, auto head, auto anySymbol) {
		template for (constexpr auto e : std::define_static_array(enumerators_of(^^State))) {
			if (state == [:e:]) {
				template for (constexpr auto a : std::define_static_array(annotations_of(e))) {
					constexpr std::meta::info type = type_of(a);
					if constexpr (has_template_arguments(type) && template_of(type) == ^^ResponseList)
						return get_response_from_list<type>(head, anySymbol);
				}
			}
		}
		throw std::runtime_error("Could not find ResponseList");
	}

	[[nodiscard]] constexpr std::span<SymbolVariant> execute_impl(bool printStates = false) {
		bool end = false;
		while (!end) {
			end = std::visit([this, printStates]<typename State>(State state) -> bool {
				using Symbol = [:impl::get_symbol<State>():];
				// Compute anySymbol (TODO: split this out into a function; duplicating reset)
				constexpr auto annotation = annotation_of<Config<Symbol>>(^^State);
				if constexpr (!annotation.has_value())
					throw std::runtime_error("Expected Config");
				auto anySymbol = annotation->anySymbol;
				
				auto response = get_response(state, head_, anySymbol);
				
				if (response.write != anySymbol)
					*head_ = response.write;

				switch (response.action) {
				case Action::Left:
					if (head_ == tape_.begin())
						throw std::runtime_error("Execution tried to move left of tape position 0");
					head_ = std::prev(head_);
					break;
				case Action::Right:
					head_ = std::next(head_);
					if (head_ == tape_.end()) {
						tape_.push_back(emptySymbol_);
						head_ = std::prev(tape_.end());
					}
				case Action::None:
					break;
				case Action::Halt:
					return true;
				default:
					std::unreachable();
				}
				
				std::visit([this, &response]<typename Enum>(Enum e){
					state_ = name_to_enum<Enum>(response.nextStateName).value();
				}, response.nextStateEnum);

				if (printStates)
					std::visit([](auto state) { std::println("{}::{}", get_scope_string(state), enum_to_string(state)); }, state_);
					
				return false;
			}, state_);
		}
		return tape_;
	}

public:
	constexpr void reset() {
		using State = [:impl::get_state_enum<Descriptor>():];
		using Symbol = [:impl::get_symbol<Descriptor>():];

		// TODO: Split this out
		constexpr auto annotation = annotation_of<Config<Symbol>>(dealias(^^State));
		if constexpr (!annotation.has_value())
			throw std::runtime_error("Expected Config");
		state_ = name_to_enum<State>(annotation->startStateName).value();

		head_ = tape_.begin();
	}

	constexpr TuringMachine() {
		// TODO: Split this out
		using State = [:impl::get_state_enum<Descriptor>():];
		using Symbol = [:impl::get_symbol<Descriptor>():];

		constexpr auto annotation = annotation_of<Config<Symbol>>(dealias(^^State));
		if constexpr (!annotation.has_value())
			throw std::runtime_error("Expected Config");
		emptySymbol_ = annotation->emptySymbol;
	}

	// TODO: Print first state if printStates == true
	// TODO: Output nicer error messages which unwrapping a non-matching state string
	[[nodiscard]] constexpr std::span<SymbolVariant> execute(std::vector<SymbolVariant> input, bool printStates = false) {
		tape_ = std::move(input);
		reset();

		return execute_impl(printStates);
	}

	[[nodiscard]] constexpr std::span<SymbolVariant> execute(bool printStates = false) {
		tape_.clear();
		tape_.resize(1, emptySymbol_);
		reset();

		return execute_impl(printStates);
	}
};

#endif //TURING_MACHINE_HPP
