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

AMS_NAMESPACE::detail
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

		/// @return The type of the message
		constexpr message_class type() noexcept
		{
			return static_cast<message_class>(
				(_type >> 4) | (_type >> 7));
		}

		/// @return The header as const buffers
		std::array<asio::const_buffer, 4> to_const_buffers() const noexcept
		{
			return
			{
				asio::buffer(&_type, 2),
				asio::buffer(&_length, 2),
				asio::buffer(&_cookie, 4),
				asio::buffer(_transaction_id)
			};
		}

		/// @return The header as mutable buffers
		std::array<asio::mutable_buffer, 4> to_buffers() noexcept
		{
			return
			{
				asio::buffer(&_type, 2),
				asio::buffer(&_length, 2),
				asio::buffer(&_cookie, 4),
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