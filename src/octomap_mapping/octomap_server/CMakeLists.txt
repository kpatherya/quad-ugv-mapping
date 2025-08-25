cmake_minimum_required(VERSION 3.5)
project(octomap_server)

# --- C++ standard / warnings ---------------------------------------------------
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- Dependencies --------------------------------------------------------------
find_package(ament_cmake_auto REQUIRED)
# If these are declared in package.xml, ament_auto_find_build_dependencies() can find them,
# but we also explicitly find key ones we link against to ensure include dirs are added.
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(octomap_msgs REQUIRED)
find_package(PCL REQUIRED)
find_package(OCTOMAP REQUIRED)

# Provide PCL & OCTOMAP headers to all targets
include_directories(
  ${PCL_INCLUDE_DIRS}
  ${OCTOMAP_INCLUDE_DIRS}
)

ament_auto_find_build_dependencies()

# Convenience list of common deps that ensure headers for ROS 2 and octomap_msgs are available
set(COMMON_DEPS
  rclcpp
  rclcpp_components
  octomap_msgs
)

# --- octomap_server_static -----------------------------------------------------
ament_auto_add_library(octomap_server_static SHARED
  src/octomap_server_static.cpp
)

ament_target_dependencies(octomap_server_static
  ${COMMON_DEPS}
)

target_link_libraries(octomap_server_static
  ${OCTOMAP_LIBRARIES}
  ${PCL_LIBRARIES}
)

rclcpp_components_register_node(octomap_server_static
  PLUGIN "octomap_server::OctomapServerStatic"
  EXECUTABLE octomap_server_static_node
)

# --- octomap_server ------------------------------------------------------------
ament_auto_add_library(octomap_server SHARED
  src/octomap_server.cpp
)

ament_target_dependencies(octomap_server
  ${COMMON_DEPS}
)

target_link_libraries(octomap_server
  ${OCTOMAP_LIBRARIES}
  ${PCL_LIBRARIES}
)

rclcpp_components_register_node(octomap_server
  PLUGIN "octomap_server::OctomapServer"
  EXECUTABLE octomap_server_node
)

# --- octomap_server_multilayer -------------------------------------------------
ament_auto_add_library(octomap_server_multilayer SHARED
  src/octomap_server.cpp
  src/octomap_server_multilayer.cpp
)

ament_target_dependencies(octomap_server_multilayer
  ${COMMON_DEPS}
)

target_link_libraries(octomap_server_multilayer
  ${OCTOMAP_LIBRARIES}
  ${PCL_LIBRARIES}
)

rclcpp_components_register_node(octomap_server_multilayer
  PLUGIN "octomap_server::OctomapServerMultilayer"
  EXECUTABLE octomap_server_multilayer_node
)

# --- tracking_octomap_server ---------------------------------------------------
ament_auto_add_library(tracking_octomap_server SHARED
  src/octomap_server.cpp
  src/tracking_octomap_server.cpp
)

ament_target_dependencies(tracking_octomap_server
  ${COMMON_DEPS}
)

target_link_libraries(tracking_octomap_server
  ${OCTOMAP_LIBRARIES}
  ${PCL_LIBRARIES}
)

rclcpp_components_register_node(tracking_octomap_server
  PLUGIN "octomap_server::TrackingOctomapServer"
  EXECUTABLE tracking_octomap_server_node
)

# --- color_octomap_server ------------------------------------------------------
ament_auto_add_library(color_octomap_server SHARED
  src/octomap_server.cpp
)

ament_target_dependencies(color_octomap_server
  ${COMMON_DEPS}
)

target_link_libraries(color_octomap_server
  ${OCTOMAP_LIBRARIES}
  ${PCL_LIBRARIES}
)

rclcpp_components_register_node(color_octomap_server
  PLUGIN "octomap_server::ColorOctomapServer"
  EXECUTABLE color_octomap_server_node
)

target_compile_definitions(color_octomap_server PUBLIC COLOR_OCTOMAP_SERVER)

# --- octomap_saver -------------------------------------------------------------
ament_auto_add_library(octomap_saver SHARED
  src/octomap_saver.cpp
)

ament_target_dependencies(octomap_saver
  rclcpp
  octomap_msgs
)

target_link_libraries(octomap_saver
  ${OCTOMAP_LIBRARIES}
)

rclcpp_components_register_node(octomap_saver
  PLUGIN "octomap_server::OctomapSaver"
  EXECUTABLE octomap_saver_node
)

# --- Tests ---------------------------------------------------------------------
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

# --- Package / install ---------------------------------------------------------
ament_auto_package(
  INSTALL_TO_SHARE
    launch
    params
)

install(PROGRAMS
  scripts/octomap_eraser_cli.py
  DESTINATION lib/${PROJECT_NAME}
)
