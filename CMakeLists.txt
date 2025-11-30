cmake_minimum_required(VERSION 3.14)
project(secure_chat_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# Helper: prefer vcpkg-provided targets when available
# -------------------------
# Try to find vcpkg/configured packages first (CONFIG)
find_package(sodium CONFIG QUIET)
find_package(cpr CONFIG QUIET)
find_package(CURL QUIET)

# -------------------------
# If sodium not found via CONFIG, fall back to pkg-config (Linux apt: libsodium-dev)
# -------------------------
if(NOT sodium_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SODIUM REQUIRED libsodium)
    message(STATUS "Using libsodium from pkg-config: ${SODIUM_VERSION}")
endif()

# -------------------------
# Ensure curl is available (needed either directly or to build CPR)
# -------------------------
# Prefer CURL target if available; otherwise try to find it
if(NOT CURL_FOUND)
    find_package(CURL REQUIRED)
endif()

# -------------------------
# If cpr wasn't found via find_package, fetch it at configure time
# (this ensures CI works even if cpr is not preinstalled)
# -------------------------
include(FetchContent)

if(NOT cpr_FOUND)
    message(STATUS "cpr not found via find_package — fetching cpr via FetchContent")
    FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        # choose a stable tag; update if you want a newer version
        GIT_TAG 1.10.0
    )
    # Make available (this will configure and build cpr as an external project)
    FetchContent_MakeAvailable(cpr)
    # After FetchContent_MakeAvailable, the target cpr::cpr should exist
    find_package(cpr CONFIG REQUIRED) # should succeed now
endif()

# -------------------------
# Create the executable
# -------------------------
add_executable(chat_client
    client/main.cpp
    client/crypto.cpp
    client/chat_client.cpp
)

# -------------------------
# Link libsodium
# - If vcpkg provided a sodium target, link that (recommended)
# - Else use pkg-config variables
# -------------------------
if(TARGET sodium::sodium)
    message(STATUS "Linking against sodium::sodium (vcpkg/configured package)")
    target_link_libraries(chat_client PRIVATE sodium::sodium)
else()
    # Use pkg-config results (SODIUM_LIBRARIES and SODIUM_INCLUDE_DIRS)
    message(STATUS "Linking against libsodium from pkg-config: ${SODIUM_LIBRARIES}")
    target_include_directories(chat_client PRIVATE ${SODIUM_INCLUDE_DIRS})
    target_link_libraries(chat_client PRIVATE ${SODIUM_LIBRARIES})
endif()

# -------------------------
# Link CPR (or fallback to CURL::libcurl)
# -------------------------
if(TARGET cpr::cpr)
    message(STATUS "Linking against cpr::cpr")
    target_link_libraries(chat_client PRIVATE cpr::cpr)
else()
    # As a fallback, link libcurl directly
    message(STATUS "cpr target not available; falling back to CURL::libcurl")
    find_package(CURL REQUIRED) # ensure it's present
    target_link_libraries(chat_client PRIVATE CURL::libcurl)
endif()

# -------------------------
# Additional compile definitions / warnings (optional)
# -------------------------
# Example: enable extra warnings on supported compilers
if(MSVC)
    target_compile_options(chat_client PRIVATE /W4 /permissive-)
else()
    target_compile_options(chat_client PRIVATE -Wall -Wextra -Wpedantic)
endif()
