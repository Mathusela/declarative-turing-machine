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

template <typename StateDescriptor, typename Symbol>
class TuringMachine {
	std::vector<Symbol> tape_;
	std::ranges::iterator_t<decltype(tape_)> head_;
	StateDescriptor state_;
	Symbol emptySymbol_;
	Symbol anySymbol_;

	template <std::meta::info responseList>
	[[nodiscard]] constexpr auto get_response_from_list() const {
		
		template for (constexpr std::meta::info responseInfo : std::define_static_array(template_arguments_of(responseList) | std::views::drop(1))) {
			constexpr Response<Symbol> response = extract<Response<Symbol>>(responseInfo);
			if (response.read == *head_ || response.read == anySymbol_)
				return response;
		}
		throw std::runtime_error("Could not find valid Response");
	}

	[[nodiscard]] constexpr auto get_response() const {
		template for (constexpr auto e : std::define_static_array(enumerators_of(^^StateDescriptor))) {
			if (state_ == [:e:]) {
				template for (constexpr auto a : std::define_static_array(annotations_of(e))) {
					constexpr std::meta::info type = type_of(a);
					if constexpr (has_template_arguments(type) && template_of(type) == ^^ResponseList) {
						return get_response_from_list<type>();
					}
				}
			}
		}
		throw std::runtime_error("Could not find ResponseList");
	}

public:
	constexpr void reset() {
		constexpr auto annotation = annotation_of<Config<Symbol>>(^^StateDescriptor);
		if constexpr (!annotation.has_value())
			throw std::runtime_error("Expected Config");
		state_ = name_to_enum<StateDescriptor>(annotation->startStateName).value();
		emptySymbol_ = annotation->emptySymbol;
		anySymbol_ = annotation->anySymbol;

		tape_.clear();
		tape_.resize(1, emptySymbol_);
		head_ = tape_.begin();
	}

	constexpr TuringMachine() {
		// constexpr auto annotation = annotation_of<Config<Symbol>>(^^StateDescriptor);
		// if constexpr (!annotation.has_value())
		// 	throw std::runtime_error("Expected Config");
		// state_ = name_to_enum<StateDescriptor>(annotation->startStateName).value();
		// emptySymbol_ = annotation->emptySymbol;
		// anySymbol_ = annotation->anySymbol;
		
		// tape_.resize(1, emptySymbol_);
		// head_ = tape_.begin();
		reset();
	}

	[[nodiscard]] constexpr std::span<Symbol> execute(std::optional<std::vector<Symbol>> input = std::nullopt, bool printStates = false) {
		if (input.has_value()) {
			tape_ = std::move(*input);
			head_ = tape_.begin();
		}

		while (true) {
			Response response = get_response();
			if (response.write != anySymbol_)
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
				return tape_;
			default:
				std::unreachable();
			}
			state_ = name_to_enum<StateDescriptor>(response.nextStateName).value();
			if (printStates)
				std::println("{}", enum_to_string(state_));
		}
	}
};

#endif //TURING_MACHINE_HPP