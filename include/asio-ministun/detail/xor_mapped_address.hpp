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

	/// @brief Get the IP address from a STUN server. Preserves the socket's non-blocking
	/// and connected state as long as the operation does not encounter an OS-level error
	/// @tparam CompletionToken The completion token type
	/// @param socket The socket to use
	/// @param endpoint The STUN server endpoint
	/// @param token The completion token
	/// @return DEDUCED. Handler must be in the form void(asio::error_code, asio::ip::udp::endpoint)
	template<typename CompletionToken>
	auto async_get_address_impl(asio::ip::udp::socket& socket,
		const asio::ip::udp::endpoint& endpoint, CompletionToken&& token)
	{
		enum class State
		{
			Connect,
			SendRequest,
			ReceiveResponse,
			ConnectOriginal,
			Cleanup,
		};
		// back-up socket traits
		error_code ignored;
		asio::ip::udp::endpoint connected_endpoint = socket.remote_endpoint(ignored);
		bool non_blocking = socket.native_non_blocking();
		// form request and response
		auto request = std::make_unique<header>(message_class::request);
		auto response = std::make_unique<xor_mapped_address>();
		return asio::async_compose<CompletionToken,
			void(asio::error_code, asio::ip::udp::endpoint)>(
				[
					&socket,
					endpoint,
					connected_endpoint,
					non_blocking,
					request = std::move(request),
					response = std::move(response),
					state = State::Connect
				]
				(
					auto& self,
					const error_code& ec = {},
					size_t bytes_transferred = 0
				) mutable {
					// make sure we don't have any errors
					if (ec)
						return self.complete(ec, asio::ip::udp::endpoint());
					switch (state)
					{
					case State::Connect:
					{
						// fetch settings
						state = State::SendRequest;
						return socket.async_connect(endpoint, std::move(self));
					}
					case State::SendRequest:
					{
						state = State::ReceiveResponse;
						return socket.async_send(request->to_const_buffers(), std::move(self));
					}
					case State::ReceiveResponse:
					{
						// check sent request
						if (bytes_transferred != request->size())
							return self.complete(asio_miniSTUN::make_error_code(errc::bad_message),
								asio::ip::udp::endpoint());
						state = State::ConnectOriginal;
						return socket.async_receive(response->to_buffers(), std::move(self));
					}
					case State::ConnectOriginal:
					{
						// check received response
						if (bytes_transferred != response->size() ||
							response->headers().type() != message_class::response_success)
							return self.complete(asio_miniSTUN::make_error_code(errc::bad_message),
								asio::ip::udp::endpoint());
						state = State::Cleanup;
						return socket.async_connect(connected_endpoint, std::move(self));
					}
					case State::Cleanup:
					{
						// restore non-blocking
						socket.native_non_blocking(non_blocking);
						// call the success handler
						return self.complete({}, asio::ip::udp::endpoint(
							response->addr(), response->port()));
					}
					}
				},
			token, socket);
	}
}

#endif