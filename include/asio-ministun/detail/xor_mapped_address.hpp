/// @file xor_mapped_address.hpp
/// @brief Implementation of fetching XOR-MAPPED-ADDRESS

#ifndef AMS_DETAIL_XOR_MAPPED_ADDRESS_H_
#define AMS_DETAIL_XOR_MAPPED_ADDRESS_H_

// AMS includes
#include <asio-ministun/detail/attributes.hpp>
#include <asio-ministun/detail/common.hpp>
#include <asio-ministun/detail/enums.hpp>

#include <iostream>

AMS_NAMESPACE::detail
{
	/// @brief A XOR-mapped address
	class xor_mapped_address : public attributes
	{
	public:
		xor_mapped_address() = default;
	private:
		uint8_t _reserved;
		uint8_t _family;
		uint16_t _xor_port;
		uint32_t _xor_addr;
	};

	/// @brief Get the IP address from a STUN server
	/// @tparam ExecutionContext The execution context
	/// @tparam CompletionToken The completion token type
	/// @param executor The executor
	/// @param endpoint The STUN server endpoint
	/// @param timeout The timeout
	/// @param token The completion token
	/// @return DEDUCED. Handler must be in the form void(std::optional<asio::ip::udp::endpoint>, asio::error_code)
	template<typename ExecutionContext, typename CompletionToken>
	auto async_get_address_impl(ExecutionContext& ctx, asio::ip::udp::endpoint endpoint,
		std::chrono::system_clock::duration timeout, CompletionToken&& token)
	{
		auto initiation = [](auto&& completion_handler,
			ExecutionContext& io_ctx,
			asio::ip::udp::endpoint stun_endpoint,
			std::chrono::system_clock::duration op_timeout)
		{
			// spawn on the executor
			asio::co_spawn(io_ctx,[completion_handler = std::forward<
				decltype(completion_handler)>(completion_handler),
				stun_endpoint, &io_ctx]() mutable->asio::awaitable<void>
				{
					// open a socket for the request
					asio::ip::udp::socket socket(io_ctx);
					socket.open(asio::ip::udp::v4());
					// construct the request
					const header request(message_class::request);
					try
					{
						co_await socket.async_send_to(request.to_const_buffers(),
							stun_endpoint, asio::use_awaitable);
					}
					catch (const std::system_error& e)
					{
						completion_handler(e.code(), stun_endpoint);
						co_return;
					}
					completion_handler({}, stun_endpoint);
			}, asio::detached);
		};
		return asio::async_initiate<CompletionToken,
			void(asio::error_code, asio::ip::udp::endpoint)>(
			initiation, token, std::ref(ctx), endpoint, timeout);
	}
}

#endif