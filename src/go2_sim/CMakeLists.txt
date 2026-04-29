cmake_minimum_required(VERSION 3.10)
project(go2_sim VERSION 4.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g)
endif()
get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
add_definitions(-DCMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)
add_definitions(-DPOLICY_DIR="${PROJECT_ROOT_DIR}/policy")

find_package(catkin REQUIRED COMPONENTS
    controller_manager
    genmsg
    joint_state_controller
    robot_state_publisher
    roscpp
    gazebo_ros
    std_msgs
    tf
    geometry_msgs
    robot_msgs
    robot_joint_controller
    rospy
)
catkin_package(
    CATKIN_DEPENDS
    robot_joint_controller
    rospy
)
add_compile_options(${GAZEBO_CXX_FLAGS})
find_package(gazebo REQUIRED)
include_directories(${catkin_INCLUDE_DIRS})

# ========================
# Inference Runtime
# ========================
set(INFERENCE_RUNTIME_DIR "${PROJECT_ROOT_DIR}/library/inference_runtime")

# LibTorch
set(USE_TORCH ON)
set(TORCH_LIBRARIES "")
set(DEFAULT_LIBTORCH_DIR "${INFERENCE_RUNTIME_DIR}/libtorch")

if(NOT DEFINED LIBTORCH_DIR OR LIBTORCH_DIR STREQUAL "")
    if(DEFINED ENV{LIBTORCH_DIR} AND EXISTS "$ENV{LIBTORCH_DIR}")
        set(LIBTORCH_DIR "$ENV{LIBTORCH_DIR}")
    elseif(EXISTS "$ENV{HOME}/libs/libtorch")
        set(LIBTORCH_DIR "$ENV{HOME}/libs/libtorch")
    else()
        set(LIBTORCH_DIR "${DEFAULT_LIBTORCH_DIR}")
    endif()
endif()
set(LIBTORCH_DIR "${LIBTORCH_DIR}" CACHE PATH "Path to the LibTorch installation")

if(EXISTS "${LIBTORCH_DIR}/include" AND EXISTS "${LIBTORCH_DIR}/lib")
    set(CMAKE_PREFIX_PATH "${LIBTORCH_DIR};${CMAKE_PREFIX_PATH}")
    find_package(Torch REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
else()
    message(FATAL_ERROR "LibTorch not found at ${LIBTORCH_DIR}. Pass -DLIBTORCH_DIR=/path/to/libtorch")
endif()

# RPATH
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${LIBTORCH_DIR}/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--disable-new-dtags")

find_package(TBB REQUIRED)
find_package(Threads REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(Python3 COMPONENTS NumPy QUIET)

if(Python3_LIBRARIES)
    get_filename_component(PYTHON_LIB_DIR "${Python3_LIBRARIES}" DIRECTORY)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${PYTHON_LIB_DIR}")
endif()

set(YAML_CPP_LIBRARIES yaml-cpp)
link_directories(/usr/local/lib)
include_directories(${YAML_CPP_INCLUDE_DIR})
include_directories(include)
add_library(inference_runtime src/inference_runtime.cpp)
target_include_directories(inference_runtime PUBLIC ${TORCH_INCLUDE_DIRS})
target_link_libraries(inference_runtime PUBLIC ${TORCH_LIBRARIES})
target_compile_definitions(inference_runtime PUBLIC USE_TORCH)
add_library(motion_loader src/motion_loader.cpp)
add_library(rl_sdk src/rl_sdk.cpp)
target_link_libraries(rl_sdk PUBLIC
    motion_loader
    inference_runtime
    Python3::Python
    Python3::Module
    TBB::tbb
)
if(Python3_NumPy_FOUND)
    target_link_libraries(rl_sdk PUBLIC Python3::NumPy)
else()
    target_compile_definitions(rl_sdk PUBLIC WITHOUT_NUMPY)
endif()

add_library(observation_buffer src/observation_buffer.cpp)
add_executable(rl_sim src/rl_sim.cpp)
target_link_libraries(rl_sim
    rl_sdk
    observation_buffer
    ${YAML_CPP_LIBRARIES}
    Threads::Threads
    ${catkin_LIBRARIES}
)
