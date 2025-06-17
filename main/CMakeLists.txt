idf_component_register(SRCS "wifi_controller.c" "webserver.c" "tasks.c" "socket.c" "scanner.c" "os_functions.c" "http_response.c" "http_request.c" "facebook_web_server.c" "dns.c" "allocator.c" "main.c"
                    INCLUDE_DIRS ".")
target_link_libraries(${COMPONENT_LIB} -Wl,-zmuldefs)