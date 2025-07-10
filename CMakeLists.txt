cmake_minimum_required (VERSION 3.20)
project (cosciev)

set (CMAKE_CXX_STANDARD 23)

add_library (cosciev INTERFACE)
target_sources (cosciev PRIVATE include/cosciev/async.hxx
                                include/cosciev/context.hxx
                                include/cosciev/event.hxx
                                include/cosciev/local_event.hxx
                                include/cosciev.hxx)
target_include_directories (cosciev INTERFACE include)
target_link_libraries (cosciev INTERFACE nvscievent)


add_executable (cosciev_test main.cxx)
target_link_libraries (cosciev_test PRIVATE cosciev)

if (ENABLE_SANITIZER)
    target_compile_options (cosciev
        BEFORE PUBLIC -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address)

    target_link_options (cosciev
        BEFORE PUBLIC -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address)
endif ()
