make_minimum_required(VERSION 3.16)

project(Simulation)

find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(shield ${sources})
target_link_libraries(shield ${Geant4_LIBRARIES})

add_custom_target(Simulation DEPENDS shield)

