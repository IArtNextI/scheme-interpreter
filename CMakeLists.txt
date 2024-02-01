set(CMAKE_CXX_STANDARD 17)

add_executable(scheme
    src/object.cpp
    src/parser.cpp
    src/scheme.cpp
    src/tokenizer.cpp
    main.cpp
)

target_include_directories(scheme PRIVATE include)

target_link_libraries(scheme PRIVATE readline)
