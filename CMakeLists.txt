cmake_minimum_required(VERSION 3.16)

project(lib6502 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB LIB6502_SOURCES CONFIGURE_DEPENDS src/*.c)

add_library(6502 STATIC ${LIB6502_SOURCES})

target_include_directories(6502 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(6502 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(6502 PRIVATE
    -Wall -Wextra -Wpedantic
)

# ---- Tests ----

include(CTest)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS test/*.c)
file(GLOB TEST_UTIL_SOURCES CONFIGURE_DEPENDS test/utils/*.c)

add_library(6502_test_utils STATIC ${TEST_UTIL_SOURCES})

target_include_directories(6502_test_utils PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/test/utils
)

target_link_libraries(6502_test_utils PRIVATE 6502)

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    add_executable(${TEST_NAME} ${TEST_SOURCE})

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        6502
        6502_test_utils
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# ---- ROMs ----

find_program(CA65 ca65)
find_program(LD65 ld65)

file(GLOB ROM_ASM_SOURCES CONFIGURE_DEPENDS test/roms/*.asm)

set(ROM_OUTPUTS)

foreach(ROM_ASM ${ROM_ASM_SOURCES})
    get_filename_component(ROM_NAME ${ROM_ASM} NAME_WE)

    set(ROM_OBJ ${CMAKE_CURRENT_BINARY_DIR}/test/roms/${ROM_NAME}.o)
    set(ROM_BIN ${CMAKE_CURRENT_BINARY_DIR}/test/roms/${ROM_NAME}.bin)
    set(ROM_LST ${CMAKE_CURRENT_BINARY_DIR}/test/roms/${ROM_NAME}.lst)

    add_custom_command(
        OUTPUT ${ROM_OBJ}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/test/roms
        COMMAND ${CA65} ${ROM_ASM} -l ${ROM_LST} -o ${ROM_OBJ}
        DEPENDS ${ROM_ASM}
        VERBATIM
    )

    add_custom_command(
        OUTPUT ${ROM_BIN}
        COMMAND ${LD65} ${ROM_OBJ}
                -C ${CMAKE_CURRENT_SOURCE_DIR}/test/roms/test-machine.cfg
                -o ${ROM_BIN}
        DEPENDS ${ROM_OBJ} ${CMAKE_CURRENT_SOURCE_DIR}/test/roms/test-machine.cfg
        VERBATIM
    )

    list(APPEND ROM_OUTPUTS ${ROM_BIN})
endforeach()

add_custom_target(test_roms
    DEPENDS ${ROM_OUTPUTS}
)