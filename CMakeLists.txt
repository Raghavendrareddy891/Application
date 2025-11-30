cmake_minimum_required(VERSION 3.14)
project(secure_chat_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Windows-focused: expect vcpkg-provided packages (cpr, sodium) ----
# When configuring locally or in CI with vcpkg, pass:
#   -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# Prefer CONFIG-mode (vcpkg) packages
find_package(cpr CONFIG QUIET)
find_package(sodium CONFIG QUIET)

# Fail early with helpful guidance if packages are missing
if(NOT cpr_FOUND)
    message(FATAL_ERROR "CPR (cpr::cpr) not found. Install via vcpkg and provide the vcpkg toolchain file.\nExample: vcpkg install cpr:x64-windows && cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg.cmake -S . -B build")
endif()

if(NOT sodium_FOUND)
    message(FATAL_ERROR "libsodium (sodium::sodium) not found. Install via vcpkg and provide the vcpkg toolchain file.\nExample: vcpkg install libsodium:x64-windows && cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg.cmake -S . -B build")
endif()

# Create target
add_executable(chat_client
    client/main.cpp
    client/crypto.cpp
    client/chat_client.cpp
)

# Link vcpkg-provided targets
target_link_libraries(chat_client
    PRIVATE
        cpr::cpr
        sodium::sodium
)

# Windows / MSVC specific compile options
if(MSVC)
    target_compile_options(chat_client PRIVATE /W4 /permissive-)
    # Define WIN32_LEAN_AND_MEAN to reduce windows headers
    target_compile_definitions(chat_client PRIVATE WIN32_LEAN_AND_MEAN)
else()
    # keep sane defaults for other compilers if accidentally used
    target_compile_options(chat_client PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: install target (useful for packaging)
install(TARGETS chat_client RUNTIME DESTINATION bin)
