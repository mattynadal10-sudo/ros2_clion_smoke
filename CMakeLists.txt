cmake_minimum_required(VERSION 3.20)
project(ros2_clion_smoke)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(hello_node src/hello_node.cpp)
target_link_libraries(hello_node PRIVATE rclcpp::rclcpp)

install(TARGETS hello_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
