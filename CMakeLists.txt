cmake_minimum_required(VERSION 3.15)
project(youtube_thread)

set(CMAKE_CXX_STANDARD 20)

if (APPLE)
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
    set(OPENSSL_LIBRARIES "/usr/local/opt/openssl/lib")
else()
    set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
    set(CMAKE_C_COMPILER o64-clang)
    set(CMAKE_CXX_COMPILER o64-clang++)
endif()

find_package(cpprestsdk REQUIRED)
find_package (OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_executable(youtube_thread main.cpp)

TARGET_LINK_LIBRARIES( youtube_thread PRIVATE cpprestsdk::cpprest OpenSSL::SSL Boost::system OpenSSL::Crypto)