cmake_minimum_required(VERSION 3.20)
project(lekid_deflection_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Geant4 REQUIRED)  # Set Geant4_DIR in CMake GUI

file(GLOB_RECURSE LEKID_SOURCES CONFIGURE_DEPENDS
     src/*.cc src/*.cpp src/*.cxx)
file(GLOB_RECURSE LEKID_HEADERS CONFIGURE_DEPENDS
     include/*.hh include/*.hpp include/*.h)

add_executable(lekid_deflection_sim ${LEKID_SOURCES} ${LEKID_HEADERS})
target_include_directories(lekid_deflection_sim PRIVATE ${Geant4_INCLUDE_DIRS} include)
target_link_libraries(lekid_deflection_sim PRIVATE ${Geant4_LIBRARIES})

if(MSVC)
  target_compile_options(lekid_deflection_sim PRIVATE /bigobj /MP)
  target_compile_definitions(lekid_deflection_sim PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
