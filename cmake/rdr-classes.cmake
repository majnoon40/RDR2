include(FetchContent)

message(STATUS "RDR-Classes")

FetchContent_Declare(
    RDR-Classes
    GIT_REPOSITORY https://github.com/YimMenu/RDR-Classes.git
    GIT_TAG        30ffb972079e1f61b7035d2dfc23d2ce642e7513
    GIT_PROGRESS TRUE
)
# The pinned RDR-Classes commit calls source_group(FILES ...) in its own
# CMakeLists, which CMake 3.29+ rejects as a hard error ("missing source
# group name"). source_group only affects IDE file grouping — never the
# compiled output — so shadow the builtin with a no-op for the duration of
# the dependency configure instead of patching or bumping the pin. The
# only other source_group call in this project (imgui.cmake) runs before
# this file is included and is unaffected.
macro(source_group)
endmacro()
FetchContent_MakeAvailable(RDR-Classes)
set_property(TARGET RDR-Classes PROPERTY CXX_STANDARD 23)