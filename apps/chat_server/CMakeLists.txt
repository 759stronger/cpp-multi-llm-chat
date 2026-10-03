pkg_check_modules(GFLAGS REQUIRED IMPORTED_TARGET gflags)
add_executable(AIChatServer chat_server.cpp main.cpp)
target_link_libraries(AIChatServer PRIVATE ai_chat_sdk PkgConfig::GFLAGS)
add_custom_command(TARGET AIChatServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/www"
        "$<TARGET_FILE_DIR:AIChatServer>/www"
    COMMENT "Copying web assets next to the chat server"
)
