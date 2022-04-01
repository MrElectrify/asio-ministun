/// @file xor_mapped_address.hpp
/// @brief Implementation of fetching XOR-MAPPED-ADDRESS

#ifndef AMS_DETAIL_XOR_MAPPED_ADDRESS_H_
#define AMS_DETAIL_XOR_MAPPED_ADDRESS_H_

// AMS includes
#include <asio-ministun/detail/attributes.hpp>
#include <asio-ministun/detail/common.hpp>
#include <asio-ministun/detail/enums.hpp>

#include <iostream>

#ifdef AMS_USE_BOOST
#include <boost/asio/experimental/awaitable_operators.hpp>
#else
#include <asio/experimental/awaitable_operators.hpp>
#endif

namespace asio_miniSTUN::detail
{
	/// @brief A XOR-mapped address
	class xor_mapped_address
	{
	public:
		xor_mapped_address() = default;

		/// @return The response address
		asio::ip::address_v4 addr() const noexcept
		{
			return asio::ip::address_v4(from_net_l(_xor_addr) ^ _header.cookie());
		}

		/// @return The headers
		const header& headers() const noexcept { return _header; }

		/// @return The response port
		uint16_t port() const noexcept
		{
			return from_net_s(_xor_port) ^ (_header.cookie() >> 16);
		}

		/// @return The size of the xor_mapped_address response
		constexpr size_t size() const noexcept
		{
			return _header.size() + _attribute.size() + sizeof(_reserved) +
				sizeof(_family) + sizeof(_xor_port) + sizeof(_xor_addr);
		}

		/// @return The mutable buffers
		std::array<asio::mutable_buffer, 10> to_buffers() noexcept
		{
			return concatenate(concatenate(_header.to_buffers(),
				_attribute.to_buffers()),
				std::array<asio::mutable_buffer, 4>{
				asio::buffer(&_reserved, sizeof(_reserved)), asio::buffer(&_family, sizeof(_family)),
					asio::buffer(&_xor_port, sizeof(_xor_port)), asio::buffer(&_xor_addr, sizeof(_xor_addr)) });
		}
	private:
		header _header;
		attributes _attribute;
		uint8_t _reserved;
		uint8_t _family;
		uint16_t _xor_port;
		uint32_t _xor_addr;
	};

	/// @brief Get the IP address from a STUN server
	/// @tparam CompletionToken The completion token type
	/// @param socket The socket to use
	/// @param endpoint The STUN server endpoint
	/// @param token The completion token
	/// @return DEDUCED. Handler must be in the form void(asio::error_code, asio::ip::udp::endpoint)
	template<typename CompletionToken>
	auto async_get_address_impl(asio::ip::udp::socket& socket,
		asio::ip::udp::endpoint endpoint, CompletionToken&& token)
	{
		auto initiation = [](auto&& completion_handler,
			asio::ip::udp::socket& local_socket,
			asio::ip::udp::endpoint stun_endpoint)
		{
			// spawn on the executor
			asio::co_spawn(local_socket.get_executor(),
				[completion_handler = std::forward<
				decltype(completion_handler)>(completion_handler),
				stun_endpoint, &local_socket]() mutable -> asio::awaitable<void>
				{
					// construct the request and response
					const header request(message_class::request);
					xor_mapped_address response;
					try
					{
						using namespace asio::experimental::awaitable_operators;
						// connect to the socket so we only receive from the target
						co_await local_socket.async_connect(stun_endpoint, asio::use_awaitable);
						const std::tuple<size_t, size_t> res = 
							co_await (local_socket.async_send(request.to_const_buffers(), asio::use_awaitable) &&
							local_socket.async_receive(response.to_buffers(), asio::use_awaitable));
						// make sure we read the expected size and all attributes are right
						if (std::get<1>(res) != response.size() ||
							response.headers().type() != message_class::response_success)
							co_return completion_handler(
								asio_miniSTUN::make_error_code(errc::bad_message),
								asio::ip::udp::endpoint());
						// read the endpoint and return it
						completion_handler({}, asio::ip::udp::endpoint(
							response.addr(), response.port()));
					}
					catch (const system_error& e)
					{
						completion_handler(e.code(), asio::ip::udp::endpoint());
					}
			}, asio::detached);
		};
		return asio::async_initiate<CompletionToken,
			void(asio::error_code, asio::ip::udp::endpoint)>(
			initiation, token, std::ref(socket), endpoint);
	}
}

#endif