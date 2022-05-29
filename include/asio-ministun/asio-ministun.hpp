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
	/// @brief Get the IP address from a STUN server. Preserves the socket's non-blocking
	/// and connected state as long as the operation does not encounter an OS-level error
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

	/// @brief Get the IP address from a STUN server. Preserves the socket's non-blocking
	/// state as long as the operation does not encounter an OS-level error. The socket must
	/// not be connected
	/// @param socket The socket to use
	/// @param endpoint The STUN server endpoint
	/// @param timeout The socket timeout
	/// @param token The completion token
	/// @return The deduced address from STUN
	asio::ip::udp::endpoint get_address(asio::ip::udp::socket& socket,
		const asio::ip::udp::endpoint& endpoint,
		const std::chrono::system_clock::duration& timeout, error_code& ec)
	{
		return detail::get_address_impl(socket, endpoint, timeout, ec);
	}

#if _WIN32
	/// @brief Get the IP address from a STUN server. Preserves the socket's non-blocking
	/// state as long as the operation does not encounter an OS-level error. The socket must
	/// not be connected. This is for a specialized use case (wine where asio assign/release
	/// is needed but not suitable)
	/// @param socket The socket to use
	/// @param endpoint The STUN server endpoint
	/// @param timeout The socket timeout
	/// @param token The completion token
	/// @return The deduced address from STUN
	asio::ip::udp::endpoint get_address(int socket,
		const asio::ip::udp::endpoint& endpoint,
		const std::chrono::system_clock::duration& timeout, error_code& ec)
	{
		return detail::get_address_impl(socket, endpoint, timeout, ec);
	}
#endif
}

#endif