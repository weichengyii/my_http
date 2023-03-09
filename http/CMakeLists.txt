aux_source_directory(./server server_srcs)
aux_source_directory(./web_socket web_socket_srcs)

add_library(http STATIC ${server_srcs} ${web_socket_srcs})

target_include_directories(http PUBLIC server web_socket)


