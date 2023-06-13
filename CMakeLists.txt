# CMakeList.txt : CMake project for MissileCommander, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("MissileCommander" LANGUAGES CXX)
set(EXECUTABLE "MissileCommander")


# add source files
file(GLOB SOURCES "Source/*.cpp")
add_executable (${EXECUTABLE} ${SOURCES})

# header files
include_directories("Include")

# Link 3rd party libraries
if(MSVC)
  target_include_directories(${EXECUTABLE} PRIVATE Deps/raylib-win64/include)
  # link static library
  target_link_libraries(${EXECUTABLE} PRIVATE ${CMAKE_SOURCE_DIR}/Deps/raylib-win64/lib/raylib.lib)
  set_target_properties(${EXECUTABLE} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"msvcrt.lib\"")
  target_link_libraries(${EXECUTABLE} PRIVATE winmm)
endif()

if (UNIX)
  target_include_directories(${EXECUTABLE} PRIVATE Deps/raylib-linux/include)
  # link static library
  target_link_libraries(${EXECUTABLE} PRIVATE ${CMAKE_SOURCE_DIR}/Deps/raylib-linux/lib/libraylib.a)
endif()
