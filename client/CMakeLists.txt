
cmake_minimum_required(VERSION 3.14)
project(secure_chat_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adjust this path if MSYS2 is installed elsewhere
include_directories("C:/msys64/mingw64/include")
link_directories("C:/msys64/mingw64/lib")

add_executable(chat_client
    main.cpp
    crypto.cpp
    chat_client.cpp
)

# libsodium + cpr from MSYS2
target_link_libraries(chat_client
    PRIVATE
        sodium
        cpr
)
