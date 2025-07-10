cmake_minimum_required (VERSION 3.20)
project (cosciev)

set (CMAKE_CXX_STANDARD 23)

add_executable (cosciev main.cxx)
target_link_libraries (cosciev nvscievent)

if (ENABLE_SANITIZER)
    target_compile_options (cosciev
        BEFORE PUBLIC -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address)

    target_link_options (cosciev
        BEFORE PUBLIC -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address)
endif ()
