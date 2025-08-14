# the minimum version of CMake.
cmake_minimum_required(VERSION 3.14)
project(NativeCase)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

# Add llama.cpp as subdirectory
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "llama: build tests" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "llama: build examples" FORCE)
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "llama: build server" FORCE)
add_subdirectory(../../../../third_party/llama.cpp ${CMAKE_BINARY_DIR}/llama.cpp)

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include
                    ../../../../third_party/llama.cpp/include
                    ../../../../third_party/llama.cpp/ggml/include)

add_library(entry SHARED 
    napi_init.cpp 
    SyncCallback/SyncCallback.cpp 
    AsyncCallback/AsyncCallback.cpp 
    AsyncPromise/AsyncPromise.cpp 
    ThreadSafeCase/ThreadSafeCase.cpp 
    LibUvCase/LibUvCase.cpp
    LlamaCppInterface/LlamaCppInterface.cpp
    LlamaCppInterface/LlamaCppNapi.cpp)

target_link_libraries(entry PUBLIC libace_napi.z.so librawfile.z.so libuv.so llama ggml)