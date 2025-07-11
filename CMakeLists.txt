cmake_minimum_required (VERSION 3.20)
project (cosciev)

add_library (cosciev STATIC)
target_compile_features (cosciev PUBLIC cxx_std_23)
target_sources (cosciev PRIVATE include/cosciev/async.hxx
                                include/cosciev/context.hxx
                                include/cosciev/event.hxx
                                include/cosciev/local_event.hxx
                                include/cosciev/main.hxx
                                include/cosciev.hxx
                                src/main.cxx)
target_include_directories (cosciev PUBLIC include)
target_link_libraries (cosciev PUBLIC nvscievent)


add_executable (cosciev_demo demo.cxx)
target_link_libraries (cosciev_demo PRIVATE cosciev)

if (ENABLE_SANITIZER)
    target_compile_options (cosciev
        BEFORE PUBLIC -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address)

    target_link_options (cosciev
        BEFORE PUBLIC -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address)
endif ()
