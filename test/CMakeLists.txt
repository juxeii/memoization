find_package(Catch2 REQUIRED)
add_executable(memoizertest test_memoize.cpp)
target_link_libraries(memoizertest Catch2::Catch2WithMain memoization)
add_test(NAME MemoizerTests COMMAND memoizertest)