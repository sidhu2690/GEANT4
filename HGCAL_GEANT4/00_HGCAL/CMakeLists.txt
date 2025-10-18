cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(Simulation)

# Find Geant4 (with UI + Visualization enabled)
find_package(Geant4 REQUIRED ui_all vis_all)

# Include Geant4 macros and settings
include(${Geant4_USE_FILE})

# Grab all source and header files in this folder
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.hh)

# Build the executable
add_executable(sim ${sources} ${headers})

# Link against Geant4
target_link_libraries(sim ${Geant4_LIBRARIES})

# Optional convenience target
add_custom_target(Simulation DEPENDS sim)
