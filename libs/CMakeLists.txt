find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Set the source files that make up the library
set(MOVIE_SRCS
   movie.h movie.c
)

# Set the library type (can be static or dynamic )
set(LIB_TYPE STATIC)

# Create the library
add_library(MovieLib ${LIB_TYPE} ${MOVIE_SRCS})