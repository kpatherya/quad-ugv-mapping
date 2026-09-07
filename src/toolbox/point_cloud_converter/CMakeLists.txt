cmake_minimum_required(VERSION 3.8)
project(point_cloud_converter)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# finding the dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

add_executable(point_cloud_converter_node src/converter.cpp)
ament_target_dependencies(point_cloud_converter_node
  rclcpp
  sensor_msgs
)

install(TARGETS point_cloud_converter_node
  DESTINATION lib/${PROJECT_NAME}
)

if (BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
