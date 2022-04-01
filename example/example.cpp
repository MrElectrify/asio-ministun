/// @file example.cpp
/// @brief A simple example of how to use asio-miniSTUN

// ASIO includes <- note that this is before AMS
#include <asio.hpp>

// AMS includes
#include <asio-ministun/asio-ministun.hpp>

// STL includes
#include <charconv>
#include <iostream>
#include <string_view>

int main(int argc, char const* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <hostname> <port>\n";
		return 1;
	}
	std::string_view hostname = argv[1];
	std::string_view port = argv[2];
	// resolve endpoint
	asio::io_context ctx;
	asio::ip::udp::resolver resolver(ctx);
	const decltype(resolver)::results_type endpoints = 
		resolver.resolve(asio::ip::udp::v4(), hostname, port);
	if (endpoints.empty() == true)
	{
		std::cerr << "Failed to resolve " << hostname << ':' << port << '\n';
		return 2;
	}
	// resolve from the first endpoint with a 2 second timeout
	std::future<asio::ip::udp::endpoint> our_endpoint_fut = asio_miniSTUN::async_get_address(
		ctx, *endpoints.begin(), std::chrono::seconds(2), asio::use_future);
	// run the ctx
	ctx.run();
	try
	{
		const asio::ip::udp::endpoint our_endpoint = our_endpoint_fut.get();
		std::cout << "Our endpoint: " << our_endpoint << '\n';
	}
	catch (const std::system_error& error)
	{
		std::cerr << "Failed to resolve our endpoint: " << error.what() << '\n';
		return 3;
	}
	return 0;
}