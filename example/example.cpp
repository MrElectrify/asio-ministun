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

using namespace asio_miniSTUN::asio;

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
	asio_miniSTUN::error_code ec;
	asio::ip::udp::resolver resolver(ctx);
	const decltype(resolver)::results_type endpoints = 
		resolver.resolve(asio::ip::udp::v4(), hostname, port, ec);
	if (endpoints.empty() == true || ec)
	{
		std::cerr << "Failed to resolve " << hostname << ':' << port << '\n';
		return 2;
	}
	// open a socket
	asio::ip::udp::socket socket(ctx, asio::ip::udp::v4());
	// resolve from the first endpoint with a 2 second timeout
	std::future<std::optional<asio::ip::udp::endpoint>> our_endpoint_fut = asio::co_spawn(ctx,
		[&socket, &endpoints, &ctx]() -> asio::awaitable<std::optional<asio::ip::udp::endpoint>> {
			using namespace asio::experimental::awaitable_operators;
			// create a timer
			asio::system_timer timer(ctx);
			timer.expires_from_now(std::chrono::seconds(2));
			const std::variant<std::monostate, asio::ip::udp::endpoint> result = 
				co_await (timer.async_wait(asio::use_awaitable) ||
					asio_miniSTUN::async_get_address(
					socket, *endpoints.begin(), asio::use_awaitable));
			co_return (result.index() == 0) ? std::nullopt : std::make_optional(std::get<1>(result));
		}, asio::use_future);
	// run the ctx
	ctx.run();
	try
	{
		const std::optional<asio::ip::udp::endpoint> our_endpoint = our_endpoint_fut.get();
		if (our_endpoint.has_value() == true)
			std::cout << "Our endpoint: " << our_endpoint.value() << '\n';
		else
			std::cerr << "Timed out fetching our endpoint\n";
	}
	catch (const asio_miniSTUN::system_error& error)
	{
		std::cerr << "Failed to resolve our endpoint: " << error.what() << '\n';
		return 3;
	}
	return 0;
}