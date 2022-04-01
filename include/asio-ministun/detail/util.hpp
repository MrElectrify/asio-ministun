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

AMS_NAMESPACE::detail
{
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

	/// @brief Converts a host long to net long
	/// @param val The host long
	/// @return The net long
	constexpr uint16_t to_net_s(uint16_t val) noexcept
	{
		if constexpr (std::endian::native == std::endian::big)
			return val;
		return val >> 8 | (val << 8) & 0xff00;
	}
	constexpr uint16_t from_net_s(uint16_t val) noexcept { return to_net_s(val); }
	/// @brief Converts a host long to net long
	/// @param val The host long
	/// @return The net long
	constexpr uint32_t to_net_l(uint32_t val) noexcept
	{
		if constexpr (std::endian::native == std::endian::big)
			return val;
		return val >> 24 |
			(val >> 8) & 0xff00 |
			(val << 8) & 0xff0000 |
			(val << 24) & 0xff000000;
	}
	constexpr uint32_t from_net_l(uint32_t val) noexcept { return to_net_l(val); }
}

#endif