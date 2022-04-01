/// @file header.hpp
/// @brief The RFC5389 STUN header

#ifndef AMS_DETAIL_HEADER_H_
#define AMS_DETAIL_HEADER_H_

// STL includes
#include <array>
#include <cstdint>

// AMS includes
#include <asio-ministun/detail/common.hpp>
#include <asio-ministun/detail/enums.hpp>
#include <asio-ministun/detail/util.hpp>

namespace asio_miniSTUN::detail
{
	/// @brief The header of a STUN message
	class header
	{
	private:
		constexpr static uint32_t COOKIE = to_net_l(0x2112A442);
	public:
		header() = default;
		/// @param msg_type The type of message
		header(message_class msg_type) noexcept : _length(0), _cookie(COOKIE) 
		{
			// binding message
			_type = to_net_s((static_cast<uint16_t>(msg_type) & 0b01) << 4 |
				(static_cast<uint16_t>(msg_type) & 0b10) << 7 | 1);
			std::fill(_transaction_id.begin(), _transaction_id.end(), 0);
		}

		/// @return The cookie used with the STUN response
		uint32_t cookie() const noexcept { return from_net_l(_cookie); }

		/// @return The size of the header
		constexpr size_t size() const noexcept
		{
			return sizeof(_type) + sizeof(_length) +
				sizeof(_cookie) + _transaction_id.size();
		}

		/// @return The type of the message
		constexpr message_class type() const noexcept
		{
			const uint16_t host_type = from_net_s(_type);
			return static_cast<message_class>(
				((host_type >> 4) & 0b01) | ((host_type >> 7)) & 0b10);
		}

		/// @return The header as const buffers
		std::array<asio::const_buffer, 4> to_const_buffers() const noexcept
		{
			return
			{
				asio::buffer(&_type, sizeof(_type)),
				asio::buffer(&_length, sizeof(_length)),
				asio::buffer(&_cookie, sizeof(_cookie)),
				asio::buffer(_transaction_id)
			};
		}

		/// @return The header as mutable buffers
		std::array<asio::mutable_buffer, 4> to_buffers() noexcept
		{
			return
			{
				asio::buffer(&_type, sizeof(_type)),
				asio::buffer(&_length, sizeof(_length)),
				asio::buffer(&_cookie, sizeof(_cookie)),
				asio::buffer(_transaction_id)
			};
		}
	private:
		uint16_t _type;
		uint16_t _length;
		uint32_t _cookie;
		std::array<uint8_t, sizeof(uint64_t) + sizeof(uint32_t)> _transaction_id;
	};
}

#endif