find_package(GTest REQUIRED)
add_executable(LLMTest test_llm.cpp)
target_link_libraries(LLMTest PRIVATE ai_chat_sdk GTest::GTest)
# Real providers and terminal input are required. Do not register this manual
# integration executable with CTest or run it automatically as an offline test.
