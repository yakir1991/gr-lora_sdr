# AddressSanitizer-friendly flags (no LTO, keep frames, debug)
set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -g -fno-omit-frame-pointer -fno-common -fsanitize=address" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address" CACHE STRING "" FORCE)
# Disable LTO – ASan doesn’t play well with it
string(REPLACE "-flto" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "-flto" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
string(REPLACE "-flto" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
