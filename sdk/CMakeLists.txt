set(SDK_SOURCES
    src/ChatGPTProvider.cpp
    src/chat_sdk.cpp
    src/DataManager.cpp
    src/DeepSeekProvider.cpp
    src/GeminiProvider.cpp
    src/LLMManager.cpp
    src/OllamaDeepSeekProvider.cpp
    src/session_manager.cpp
    src/util/my_logger.cpp
)
add_library(ai_chat_sdk STATIC ${SDK_SOURCES})
target_include_directories(ai_chat_sdk PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(ai_chat_sdk PUBLIC
    cpp_httplib_headers
    PkgConfig::JSONCPP
    PkgConfig::FMT
    PkgConfig::SPDLOG
    PkgConfig::SQLITE3
)
install(TARGETS ai_chat_sdk ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")
