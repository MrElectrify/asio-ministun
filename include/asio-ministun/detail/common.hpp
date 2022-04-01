/// @file common.hpp
/// @brief Common utilities and macros

#ifndef AMS_DETAIL_COMMON_H_
#define AMS_DETAIL_COMMON_H_

namespace asio_miniSTUN
{
#ifdef AMS_USE_BOOST
	namespace asio = boost::asio;
	using errc = boost::system::errc::errc_t;
	using error_code = boost::system::error_code;
	using system_error = boost::system::system_error;
#else
	namespace asio = ::asio;
	using std::errc;
	using error_code = asio::error_code;
	using system_error = asio::system_error;
#endif
	/// @brief Creates error code value for errc enum e
	/// @param e The error code enum to create error for
	/// @return The error code
	error_code make_error_code(errc e) noexcept
	{
#ifdef AMS_USE_BOOST
		return boost::system::make_error_code(e);
#else
		return std::make_error_code(e);
#endif
	}
}

#endif