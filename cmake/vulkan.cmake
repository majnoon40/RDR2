include(FetchContent)

message(STATUS "Setting up Vulkan Headers")
FetchContent_Declare(
    Vulkan
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG sdk-1.1.108.0 # matches RDR2's SDK
    GIT_PROGRESS TRUE
)

set(CMAKE_WARN_DEPRECATED 0 CACHE INTERNAL "Suppress deprecation shortly")
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
# Vulkan-Headers at this SDK era declares cmake_minimum_required(VERSION
# 3.0.x), which CMake 3.31+ hard-rejects ("Compatibility with CMake < 3.5
# has been removed"). This officially supported variable relaxes the check
# for dependencies fetched by this project.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MakeAvailable(Vulkan)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 0 CACHE INTERNAL "Re-enable dev warnings")
set(CMAKE_WARN_DEPRECATED 1 CACHE INTERNAL "Restore deprecation notices")
