/// @file common.hpp
/// @brief Common utilities and macros

#ifndef AMS_DETAIL_COMMON_H_
#define AMS_DETAIL_COMMON_H_

#define AMS_NAMESPACE namespace asio_miniSTUN
#ifdef AMS_USE_BOOST
AMS_NAMESPACE
{
	using namespace boost;
}
#endif

#endif