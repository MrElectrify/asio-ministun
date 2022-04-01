add_executable(example example.cpp)

target_link_libraries(example
	PRIVATE asio
	PRIVATE asio-ministun)