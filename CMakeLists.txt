cmake_minimum_required(VERSION 3.14)
project(secure_chat_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# Try to find vcpkg-provided (CONFIG) packages first
# -------------------------
find_package(sodium CONFIG QUIET)
find_package(cpr CONFIG QUIET)
find_package(CURL QUIET)

# -------------------------
# If sodium not found via CONFIG, fall back to pkg-config (MSYS2 / apt)
# -------------------------
if(NOT sodium_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SODIUM REQUIRED libsodium)
    message(STATUS "Using libsodium from pkg-config: ${SODIUM_VERSION}")
endif()

# -------------------------
# Ensure libcurl is available (needed by cpr or as fallback)
# -------------------------
if(NOT CURL_FOUND)
    find_package(CURL REQUIRED)
endif()

# -------------------------
# If cpr wasn't found via find_package, bring it in via FetchContent
# -------------------------
include(FetchContent)
if(NOT cpr_FOUND)
    message(STATUS "cpr not found via find_package — fetching cpr via FetchContent")
    FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 1.10.0
    )
    # This will create target cpr::cpr (if it configures successfully)
    FetchContent_MakeAvailable(cpr)
    # Try to find again
    find_package(cpr CONFIG QUIET)
endif()

# -------------------------
# Create executable
# -------------------------
add_executable(chat_client
    client/main.cpp
    client/crypto.cpp
    client/chat_client.cpp
)

# -------------------------
# Link libsodium (vcpkg target preferred, otherwise pkg-config)
# -------------------------
if(TARGET sodium::sodium)
    message(STATUS "Linking with sodium::sodium (vcpkg or config package)")
    target_link_libraries(chat_client PRIVATE sodium::sodium)
else()
    message(STATUS "Linking with libsodium from pkg-config")
    target_include_directories(chat_client PRIVATE ${SODIUM_INCLUDE_DIRS})
    target_link_libraries(chat_client PRIVATE ${SODIUM_LIBRARIES})
endif()

# -------------------------
# Link cpr (preferred) or fallback to CURL::libcurl
# -------------------------
if(TARGET cpr::cpr)
    message(STATUS "Linking with cpr::cpr")
    target_link_libraries(chat_client PRIVATE cpr::cpr)
else()
    message(STATUS "cpr not available; falling back to CURL::libcurl")
    find_package(CURL REQUIRED)
    target_link_libraries(chat_client PRIVATE CURL::libcurl)
endif()

# -------------------------
# Compile options
# -------------------------
if(MSVC)
    target_compile_options(chat_client PRIVATE /W4 /permissive-)
else()
    target_compile_options(chat_client PRIVATE -Wall -Wextra -Wpedantic)
endif()
