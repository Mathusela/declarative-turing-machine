#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <meta>
#include <optional>
#include <string_view>

template <typename T>
consteval std::optional<T> annotation_of(std::meta::info x) {
	for (std::meta::info a : annotations_of(x)) {
		if (type_of(a) == ^^T)
			return extract<T>(a);
	}
	return std::nullopt;
}

template <typename E>
	requires(std::is_enum_v<E>)
[[nodiscard]] constexpr std::optional<E> name_to_enum(std::string_view str) noexcept {
	template for (constexpr auto e : std::define_static_array(enumerators_of(^^E))) {
		if (identifier_of(e) == str)
			return [:e:];
	}
	return std::nullopt;
}

template <typename E>
[[nodiscard]] constexpr std::string_view enum_to_string(E x) {
	template for (constexpr auto e : std::define_static_array(enumerators_of(^^E))) {
		if ([:e:] == x)
			return identifier_of(e);
	}
	return "";
}

namespace structural {
    struct StringView {
        const char* data;
        std::size_t size;

        constexpr StringView() = default;
		
		template <std::size_t n>
		constexpr StringView(const char(&str)[n])
            : data(std::define_static_string(str)), size(n-1) {}
        
        constexpr operator std::string_view() noexcept {
            return {data, size};
        }
        constexpr operator const std::string_view() const noexcept {
            return {data, size};
        }
    };
}

#endif // UTILITY_HPP