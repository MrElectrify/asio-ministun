/// @file enums.hpp
/// @brief STUN Enums

#ifndef AMS_ENUMS_H_
#define AMS_ENUMS_H_

// AMS includes
#include <asio-ministun/detail/common.hpp>

// STL includes
#include <cstdint>

namespace asio_miniSTUN::detail
{
	/// @brief The message class
	enum class message_class
	{
		request = 0b00,
		indication = 0b01,
		response_success = 0b10,
		response_error = 0b11,
	};

	enum class message_method
	{
		binding = 0b01,
	};

	enum class message_type
	{
		mapped_address = 0x0001,
		message_integrity = 0x0008,
		error_code = 0x0009,
		unknown_attributes = 0x000a,
		realm = 0x0014,
		none = 0x0015,
		xor_mapped_address = 0x0020,
	};
}

#endif