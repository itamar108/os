#cmake_minimum_required(VERSION 3.1)
#
#
## NOTE: You can't have both ThreadSanitizer and AddressSanitizer enabled at the same time.
#
## Uncomment the following to enable ThreadSanitizer.
##set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
##set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
#
## Uncomment the following to enable AddressSanitizer.
##set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
##set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#
#
## Project configuration
#project(ex4 VERSION 1.0 LANGUAGES C CXX)
#
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#
#
################################################################
##add_executable(ex4 flowExample.cpp)
######################################################
#
#add_library(VirtualMemory
#        VirtualMemory.cpp VirtualMemory.h
#        PhysicalMemory.cpp PhysicalMemory.h
#        # ------------- Add your own .h/.cpp files here -------------------
#        )
################################################################
##target_link_libraries(ex4 PRIVATE VirtualMemory)
######################################################
#
#
#set_property(TARGET VirtualMemory PROPERTY CXX_STANDARD 11)
#target_compile_options(VirtualMemory PUBLIC -Wall)
#target_include_directories(VirtualMemory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
################################################################
##target_compile_options(ex4 PUBLIC -Wall -Wextra)
######################################################
#
## Add tests
#add_subdirectory(mattanTests)


#+=============================================
#+=============================================
#+=============================================
#+=============================================



cmake_minimum_required(VERSION 3.1)



# Project configuration
project(ex4 VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------- You may only modify the following section ----------- #
set(vm_source_files
        VirtualMemory.h VirtualMemory.cpp
        PhysicalMemory.h PhysicalMemory.cpp
        MemoryConstants.h

        # add your own files here
        )

set(vm_compile_options -Wall -Wextra -g -O2)
#set(vm_compile_options -Wall -g)


# ------------ Do not modify anything below these lines ------------- #

set(vm_link_libraries)

set(vm_compile_definitions INC_TESTING_CODE)

set(USE_SPEEDLOG OFF)

if(USE_SPEEDLOG)
    add_subdirectory(spdlog)
    list(APPEND vm_link_libraries spdlog::spdlog)
    list(APPEND vm_compile_definitions USE_SPEEDLOG)
endif()


function(createVMTarget targetName define)
    add_library(${targetName} ${vm_source_files})
    set_property(TARGET ${targetName} PROPERTY CXX_STANDARD 11)
    target_compile_options(${targetName} PUBLIC ${vm_compile_options})
    target_link_libraries(${targetName} PUBLIC ${vm_link_libraries})
    target_compile_definitions(${targetName} PUBLIC ${vm_compile_definitions} ${define})
endfunction()


# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# ---------------- You may modify the following -----------------
# You can comment out some of these to speedup compilation,
# but make sure you comment the corresponding lines in tests/CMakeLists.txt

createVMTarget(VirtualMemory NORMAL_CONSTANTS)
createVMTarget(TestVirtualMemory TEST_CONSTANTS)
createVMTarget(OffsetDifferentThanIndexMemory OFFSET_DIFFERENT_FROM_INDEX)
createVMTarget(SingleTableVirtualMemory SINGLE_TABLE_CONSTANTS)
createVMTarget(UnreachableFramesVirtualMemory UNREACHABLE_FRAMES_CONSTANTS)
createVMTarget(NoEvictionVirtualMemory NO_EVICTION_CONSTANTS)

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# Add tests
add_subdirectory(tests)



#
#
#cmake_minimum_required(VERSION 3.1)
#
#
## NOTE: You can't have both ThreadSanitizer and AddressSanitizer enabled at the same time.
#
## Uncomment the following to enable ThreadSanitizer.
##set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
##set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
#
## Uncomment the following to enable AddressSanitizer.
##set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
##set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#
#
## Project configuration
#project(ex4 VERSION 1.0 LANGUAGES C CXX)
#
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#
#
################################################################
#add_executable(ex4 SimpleTest.cpp)
######################################################
#
#add_library(VirtualMemory
#        VirtualMemory.cpp VirtualMemory.h
#        PhysicalMemory.cpp PhysicalMemory.h
#        # ------------- Add your own .h/.cpp files here -------------------
#        )
################################################################
#target_link_libraries(ex4 PRIVATE VirtualMemory)
######################################################
#
#
#set_property(TARGET VirtualMemory PROPERTY CXX_STANDARD 11)
#target_compile_options(VirtualMemory PUBLIC -Wall)
#target_include_directories(VirtualMemory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
################################################################
#target_compile_options(ex4 PUBLIC -Wall)
######################################################
#
