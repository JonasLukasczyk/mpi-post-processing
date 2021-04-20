cmake_minimum_required(VERSION 3.2)
project(proxy_pnf)

find_package(MPI REQUIRED)
find_package(VTK REQUIRED)
find_package(TTKVTK REQUIRED)

include_directories(
  ./
  ${MPI_INCLUDE_PATH}
  ${VTK_INCLUDE_PATH}
)

set(SOURCES main.cpp PerlinNoise.cpp PostProcessing.cpp)

add_executable(proxy_pnf ${SOURCES})

target_link_libraries(proxy_pnf
  ${MPI_LIBRARIES}
  ${VTK_LIBRARIES}
  ttk::vtk::ttkAll
)

if(MPI_COMPILE_FLAGS)
  set_target_properties(proxy_pnf PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(proxy_pnf PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()
