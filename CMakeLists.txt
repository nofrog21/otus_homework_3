cmake_minimum_required(VERSION 3.14)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(my_allocator VERSION ${PROJECT_VERSION})

configure_file(version.h.in version.h)

add_executable(my_allocator main.cpp)

set_target_properties(my_allocator PROPERTIES
  CXX-STANDARD 14
  CXX-STANDARD-REQUIRED ON
)

target_include_directories(my_allocator PRIVATE "${CMAKE_BINARY_DIR}")

if (MSVC)
    target_compile_options(my_allocator PRIVATE
        /W4
    )
else ()
    target_compile_options(my_allocator PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
endif()

install(TARGETS my_allocator RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT ivansannikov29@gmail.com)
include(CPack)
