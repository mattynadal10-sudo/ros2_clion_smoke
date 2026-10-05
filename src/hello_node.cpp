#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class HelloNode final : public rclcpp::Node {
public:
  HelloNode() : Node("hello_node") {
    timer_ = create_wall_timer(1s, [this] {
      RCLCPP_INFO(get_logger(), "ROS 2 Lyrical + C++ + CLion are ready");
    });
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<HelloNode>());
  rclcpp::shutdown();
  return 0;
}
