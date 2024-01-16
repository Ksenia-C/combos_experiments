set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/home/ubuntu/combos_cpp_shadow_new/simgrid-3.34")

find_package(SimGrid REQUIRED)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master
)
FetchContent_GetProperties(yaml-cpp)

if(NOT yaml-cpp_POPULATED)
  message(STATUS "Fetching yaml-cpp...")
  FetchContent_Populate(yaml-cpp)
  add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
endif()


set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.45.0 COMPONENTS context serialization) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 


# find_package(SimGrid REQUIRED)
    include_directories(${SimGrid_INCLUDE_DIR})


    include_directories("${PROJECT_SOURCE_DIR}/Files")


    add_executable(boinc_sim_cpp boinc.cpp rand.cpp components/scheduler.cpp
                   components/data_server.cpp components/shared.cpp
                   components/data_client_server.cpp client_side/data_client.cpp
                   client_side/fetch_work.cpp client_side/execute_task.cpp)
    target_link_libraries(boinc_sim_cpp ${SimGrid_LIBRARY} ${Boost_LIBRARIES} yaml-cpp)

endif()