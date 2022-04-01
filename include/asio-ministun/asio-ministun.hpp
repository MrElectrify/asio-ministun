/// @file asio-ministun.hpp
/// @brief Convenience header

#ifndef AMS_ASIOMINISTUN_HPP_H_
#define AMS_ASIOMINISTUN_HPP_H_

// AMS includes
#include <asio-ministun/detail/common.hpp>
#include <asio-ministun/detail/xor_mapped_address.hpp>

// STL includes
#include <optional>
#include <utility>

namespace asio_miniSTUN
{
	/// @brief Get the IP address from a STUN server
	/// @tparam CompletionToken The completion token type
	/// @param socket The socket to use
	/// @param endpoint The STUN server endpoint
	/// @param token The completion token
	/// @return DEDUCED. Handler must be in the form void(asio::error_code, asio::ip::udp::endpoint)
	template<typename CompletionToken>
	auto async_get_address(asio::ip::udp::socket& socket,
		const asio::ip::udp::endpoint& endpoint, CompletionToken&& token)
	{
		return detail::async_get_address_impl(socket, endpoint,
			std::forward<CompletionToken>(token));
	}
}

#endif