cmake_minimum_required(VERSION 3.8)

project(ASIO-MiniSTUN
	VERSION 0.1
	DESCRIPTION "ASIO Mini-STUN Client"
	LANGUAGES CXX)

# Let the user pick which version of asio to use
option(AMS_USE_BOOST "Use boost::asio versus standalone asio" OFF)
option(AMS_BUILD_EXAMPLE "Build the asio-multiSTUN example" OFF)

# Create the target
add_library(asio-ministun INTERFACE)
target_include_directories(asio-ministun INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(asio-ministun INTERFACE cxx_std_20)
if (AMS_USE_BOOST)
	target_compile_definitions(asio-ministun INTERFACE AMS_USE_BOOST=1)
endif()

if (AMS_BUILD_EXAMPLE)
	# You must set an asio path for examples
	set(AMS_ASIO_INCLUDE_DIR "" CACHE PATH "asio Include directory. If there is already an asio target, this is ignored")

	if ((NOT TARGET asio) AND
		AMS_ASIO_INCLUDE_DIR STREQUAL "")
		message(FATAL_ERROR "asio must be provided as a target or in CMA_ASIO_INCLUDE_DIR")
	endif()

	# if an asio target has not been defined, define one
	if (NOT TARGET asio)
		add_library(asio INTERFACE)
		target_include_directories(asio
			INTERFACE ${AMS_ASIO_INCLUDE_DIR})
	endif()
	add_subdirectory(example)
endif()
