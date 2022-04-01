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

AMS_NAMESPACE
{
	/// @brief Get the IP address from a STUN server
	/// @tparam ExecutionContext The execution context
	/// @tparam CompletionToken The completion token type
	/// @param executor The executor
	/// @param endpoint The STUN server endpoint
	/// @param timeout The timeout
	/// @param token The completion token
	/// @return DEDUCED. Handler must be in the form void(std::optional<asio::ip::udp::endpoint>, asio::error_code)
	template<typename ExecutionContext, typename CompletionToken>
	auto async_get_address(ExecutionContext& ctx, asio::ip::udp::endpoint endpoint,
		std::chrono::system_clock::duration timeout, CompletionToken&& token)
	{
		return detail::async_get_address_impl(ctx, endpoint, timeout,
			std::forward<CompletionToken>(token));
	}
}

#endif