# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(assignment3)

# Libs
#add_subdirectory(libs)
#list(APPEND EXTRA_LIBS MovieLib)
#list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/libs")
#target_link_libraries(app PUBLIC ${EXTRA_LIBS})
#target_include_directories(app PUBLIC ${EXTRA_INCLUDES})


FILE(GLOB app_sources src/*.c libs/*.c)
target_sources(app PRIVATE ${app_sources})


#target_sources(app PRIVATE src/main.c)



