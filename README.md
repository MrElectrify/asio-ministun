# asio-miniSTUN
asio-miniSTUN is a MINI (read: incomplete) STUN client designed to simply fetch an IP per RFC 5389

## Compatibility
asio-miniSTUN was designed for [standalone asio](https://github.com/chriskohlhoff/asio) but should work with `boost::asio` with the CMake or define `AMS_USE_BOOST`, although the compatibility has not been tested. If you do use it for `boost::asio`, kindly make a PR with any type aliasing that needs to happen to make it work.

asio-miniSTUN requires a C++20 compiler

## Usage
asio-miniSTUN comes with one simple function - `async_get_address`. It comes with the signature `DEDUCED(asio::ip::udp::socket& local_socket, const asio::ip::udp::endpoint& stun_endpoint, CompletionToken)`, which uses a local UDP socket and STUN endpoint to perform a XOR-MAPPED-ADDRESS request. An example is provided.
