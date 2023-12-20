cmake_minimum_required(VERSION 3.8)
project(can_sender)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Set the path to the can-utils library
set(CAN_UTILS_INCLUDE_DIRS "/home/devuser/AOX/ROS2/src/can-utils")

# Find can-utils headers
#find_path(CAN_UTILS_INCLUDE_DIRS NAMES dbc/dbc_parser.h PATHS ${CAN_UTILS_PATH}/include)

# Check if include directories are found
if(NOT CAN_UTILS_INCLUDE_DIRS)
    message(FATAL_ERROR "can-utils headers not found. Set CAN_UTILS_PATH to the correct location.")
endif()
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
endif()

# Find can-utils library
#find_library(CAN_UTILS_LIB NAMES dbc PATHS ${CAN_UTILS_PATH}/lib)

# Check if the library is found
#if(NOT CAN_UTILS_LIB)
#    message(FATAL_ERROR "can-utils library not found. Set CAN_UTILS_PATH to the correct location.")
#endif()

# Include can-utils headers
include_directories(${CAN_UTILS_INCLUDE_DIRS})

# Your ROS 2 dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(can_msgs REQUIRED)
find_package(aox_can_msgs REQUIRED)
find_package(std_msgs REQUIRED)
# Add your executable
add_executable(sender src/sender.cpp ${CAN_UTILS_INCLUDE_DIRS}/dbc/dbc_parser.cpp ${CAN_UTILS_INCLUDE_DIRS}/v2c/v2c_transcoder.cpp)

# Link against can-utils library
# target_link_libraries(sender ${CAN_UTILS_LIB})

# Link against ROS 2 libraries
ament_target_dependencies(sender rclcpp can_msgs aox_can_msgs std_msgs)

# Set the C++ standard specifically for your target
target_compile_features(sender PRIVATE cxx_std_20)
# Install the sender
install(TARGETS
sender
  DESTINATION lib/${PROJECT_NAME})

# # Install other files if needed (e.g., launch files)
# install(DIRECTORY
#   launch
#   DESTINATION share/${PROJECT_NAME})

ament_package()