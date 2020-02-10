cmake_minimum_required(VERSION 2.8.4)
project(germanBot)

set(CURL_LIBRARY "-lcurl -lboost_locale")
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED locale REQUIRED)
find_package(CURL)
include_directories(/usr/local/include ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()

add_executable(germanBot src/main.cpp src/auth.hpp src/langtool.hpp)

target_link_libraries(germanBot /usr/local/lib/libTgBot.a ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${CURL_LIBRARY} ${CURL_LIBRARIES})
