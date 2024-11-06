add_library(toolbox
    Utility.cpp
    SkipList.cpp
    )

configure_file(Version.h.in Version.h)

target_include_directories(
    toolbox PUBLIC
    "${PROJECT_BINARY_DIR}/../src/"
)