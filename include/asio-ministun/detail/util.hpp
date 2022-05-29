/// @file util.hpp
/// @brief Utilities

#ifndef AMS_DETAIL_UTIL_H_
#define AMS_DETAIL_UTIL_H_

// AMS includes
#include <asio-ministun/detail/common.hpp>

// STL includes
#include <array>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <type_traits>

namespace asio_miniSTUN::detail
{
	/// @brief Collects a range into a container
	/// @tparam C The container type to collect into
	/// @tparam R The range type
	/// @param r The range
	/// @return The container with the range
	template<typename C, typename R>
	C collect(R&& r)
	{
		C result;
		std::ranges::copy(std::forward<R>(r), std::back_inserter(result));
		return result;
	}

	/// @brief Concatenates arrays
	/// @tparam Type The element type
	/// @tparam sizes The sizes of the arrays
	/// @param arrays The arrays
	/// @return The concatenated arrays
	template <typename Type, std::size_t... sizes>
	auto concatenate(const std::array<Type, sizes>&... arrays)
	{
		std::array<Type, (sizes + ...)> result;
		std::size_t index{};

		((std::copy_n(arrays.begin(), sizes, result.begin() + index), index += sizes), ...);

		return result;
	}

	/// @brief Converts a host integral to net integral
	/// @tparam T The integral type
	/// @param val The host long
	/// @return The net long
	template<typename T>
	inline constexpr T to_net(T val) noexcept requires(std::is_integral_v<T> && sizeof(T) == 4)
	{
		if constexpr (std::endian::native == std::endian::big)
			return val;
		return (val & 0xff) << 24 |
			((val >> 8) & 0xff) << 16 |
			((val >> 16) & 0xff) << 8 |
			(val >> 24) & 0xff;
	}
	/// @brief Converts a host integral to net integral
	/// @tparam T The integral type
	/// @param val The host long
	/// @return The net long
	template<typename T>
	inline constexpr T to_net(T val) noexcept requires(std::is_integral_v<T> && sizeof(T) == 2)
	{
		if constexpr (std::endian::native == std::endian::big)
			return val;
		return (val & 0xff) << 8 |
			(val >> 8) & 0xff;
	}
	/// @brief Converts a net long to host long
	/// @param val The net long
	/// @return The host long
	template<typename T>
	inline constexpr T from_net(T val) noexcept requires(std::is_integral_v<T>) { return to_net(val); }
}

#endif