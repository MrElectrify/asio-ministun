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
			return asio::ip::address_v4(from_net(_xor_addr) ^ _header.cookie());
		}

		/// @return The headers
		const header& headers() const noexcept { return _header; }

		/// @return The response port
		uint16_t port() const noexcept
		{
			return from_net(_xor_port) ^ (_header.cookie() >> 16);
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
	/// state as long as the operation does not encounter an OS-level error. The socket must
	/// not be connected
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
			SendRequest,
			ReceiveResponse,
			Cleanup,
		};
		// back-up socket traits
		error_code ignored;
		bool non_blocking = socket.native_non_blocking();
		// form request and response
		auto request = std::make_unique<header>(message_class::request);
		auto response = std::make_unique<xor_mapped_address>();
		auto recv_endpoint = std::make_unique<asio::ip::udp::endpoint>();
		return asio::async_compose<CompletionToken,
			void(asio::error_code, asio::ip::udp::endpoint)>(
				[
					&socket,
					endpoint,
					non_blocking,
					request = std::move(request),
					response = std::move(response),
					recv_endpoint = std::move(recv_endpoint),
					state = State::SendRequest
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
					case State::SendRequest:
					{
						// set non-blocking
						error_code ignored;
						socket.native_non_blocking(true);
						// ensure the socket is not already connected
						if (socket.remote_endpoint(ignored); !ignored)
							return self.complete(asio_miniSTUN::make_error_code(
								errc::already_connected), asio::ip::udp::endpoint());
						state = State::ReceiveResponse;
						return socket.async_send_to(request->to_const_buffers(), endpoint, std::move(self));
					}
					case State::ReceiveResponse:
					{
						// check sent request
						if (bytes_transferred != request->size())
							return self.complete(asio_miniSTUN::make_error_code(errc::bad_message),
								asio::ip::udp::endpoint());
						state = State::Cleanup;
						return socket.async_receive_from(response->to_buffers(), *recv_endpoint, std::move(self));
					}
					case State::Cleanup:
					{
						// ignore unexpected responses
						if (*recv_endpoint != endpoint)
							return socket.async_receive_from(response->to_buffers(), *recv_endpoint, std::move(self));
						// check received response
						if (bytes_transferred != response->size() ||
							response->headers().type() != message_class::response_success)
							return self.complete(asio_miniSTUN::make_error_code(errc::bad_message),
								asio::ip::udp::endpoint());
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

	/// @brief Get the IP address from a STUN server. Preserves the socket's non-blocking
	/// state as long as the operation does not encounter an OS-level error. The socket must
	/// not be connected
	/// @param socket The socket to use
	/// @param endpoint The STUN server endpoint
	/// @param timeout The socket timeout
	/// @param token The completion token
	/// @return The deduced address from STUN
	asio::ip::udp::endpoint get_address_impl(asio::ip::udp::socket& socket,
		const asio::ip::udp::endpoint& endpoint, 
		const std::chrono::system_clock::duration& timeout, error_code& ec)
	{
		// ensure the socket is not already connected
		if (socket.remote_endpoint(ec); !ec)
			return {};
		bool non_blocking = socket.native_non_blocking();
		// get old timeout
		asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO> rcv_timeout;
		if (socket.get_option(rcv_timeout, ec))
			return {};
		// set the new timeout
		if (socket.set_option(asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>(
			static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count())), ec))
			return {};
		header request(message_class::request);
		// disable non-blocking
		socket.native_non_blocking(false);
		// send the request
		if (socket.send_to(request.to_const_buffers(), endpoint, 0, ec); ec)
			return {};
		// receive the response until we get it from the STUN server
		xor_mapped_address response;
		for (asio::ip::udp::endpoint recv_endpoint; recv_endpoint != endpoint;)
		{
			if (socket.receive_from(response.to_buffers(), recv_endpoint, 0, ec); ec)
				return {};
		}
		// return non-blocking and recv timeout
		if (socket.native_non_blocking(non_blocking, ec) ||
			socket.set_option(rcv_timeout, ec))
			return {};
		return asio::ip::udp::endpoint(response.addr(), response.port());
	}
}

#endif