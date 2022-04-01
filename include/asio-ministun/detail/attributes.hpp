/// @file attributes.hpp
/// @brief The RFC5389 STUN attributes

#ifndef AMS_DETAIL_ATTRIBUTES_H_
#define AMS_DETAIL_ATTRIBUTES_H_

// STL includes
#include <array>

// AMS includes
#include <asio-ministun/detail/common.hpp>
#include <asio-ministun/detail/enums.hpp>
#include <asio-ministun/detail/header.hpp>
#include <asio-ministun/detail/util.hpp>

namespace asio_miniSTUN::detail
{
	/// @brief The STUN attributes
	class attributes
	{
	public:
		attributes() = default;
		/// @param msg_type The message type
		attributes(message_type msg_type, uint16_t length) noexcept :
			_msg_type(to_net_s(static_cast<uint16_t>(msg_type))),
			_length(to_net_s(length)) {}

		/// @return The size of an attribute
		constexpr size_t size() const noexcept { return sizeof(_msg_type) + sizeof(_length); }

		/// @return The attribute const buffers
		std::array<asio::const_buffer, 2> to_const_buffers() const noexcept
		{
			return 
			{ 
				asio::buffer(&_msg_type, 2), 
				asio::buffer(&_length, 2) 
			};
		}

		/// @return The attribute const buffers
		std::array<asio::mutable_buffer, 2> to_buffers() noexcept
		{
			return 
			{
				asio::buffer(&_msg_type, 2),
				asio::buffer(&_length, 2)
			};
		}

		/// @return The message type
		message_type type() const noexcept { return static_cast<message_type>(from_net_s(_msg_type)); }
	private:
		uint16_t _msg_type;
		uint16_t _length;
	};
}

#endif