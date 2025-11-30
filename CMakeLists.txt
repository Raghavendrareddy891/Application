cmake_minimum_required(VERSION 3.14)
project(secure_chat_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# Find libsodium
#   - Linux (apt): uses pkg-config (libsodium-dev)
#   - Windows (vcpkg): provides sodium via find_package(sodium)
# ------------------------------------------------------------------------------

# Try vcpkg-style package first
find_package(sodium QUIET)

if(NOT sodium_FOUND)
    # Fallback for Linux: pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SODIUM REQUIRED libsodium)
    include_directories(${SODIUM_INCLUDE_DIRS})
    link_directories(${SODIUM_LIBRARY_DIRS})
endif()

# ------------------------------------------------------------------------------
# Find CPR (libcurl based HTTP library)
#   - Windows (vcpkg): find_package(cpr CONFIG REQUIRED)
#   - Linux: CPR is header-only when installed via apt, so we just link curl
# ------------------------------------------------------------------------------

find_package(cpr CONFIG QUIET)
find_package(CURL REQUIRED)

# ------------------------------------------------------------------------------
# Build target
# ------------------------------------------------------------------------------

add_executable(chat_client
    client/main.cpp
    client/crypto.cpp
    client/chat_client.cpp
)

# ------------------------------------------------------------------------------
# Link libraries cross-platform
# ------------------------------------------------------------------------------

# Sodium library
if(sodium_FOUND)
    target_link_libraries(chat_client PRIVATE sodium)
else()
    target_link_libraries(chat_client PRIVATE ${SODIUM_LIBRARIES})
endif()

# CPR or fallback to curl
if(cpr_FOUND)
    target_link_libraries(chat_client PRIVATE cpr::cpr)
else()
    target_link_libraries(chat_client PRIVATE CURL::libcurl)
endif()
