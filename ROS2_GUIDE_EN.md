# ROS 2 for Mobile Robotics: Concepts and C++ Workflow

An English study guide based on the two *Control of Mobile Robots* lecture decks, adapted to this computer's ROS 2 Lyrical environment. The lecture examples use ROS 2 Jazzy and Ubuntu 24.04; the core concepts carry over, while the commands below use this machine's installed path and workspace.

## 1. What ROS 2 is

ROS 2 is a robotics software framework: libraries, middleware, command-line tools, and reusable packages. It is not an operating system. It helps separate robot software into components that can be developed and run independently, while providing standard ways to exchange data.

Typical capabilities include:

- hardware and device interfaces;
- communication between processes and computers;
- parameters, logging, launch, visualization, and recording tools;
- reusable packages for sensing, control, mapping, planning, and simulation.

ROS 2 uses a distributed graph of nodes. DDS-based middleware handles discovery and transport; there is no single ROS master process that every node must contact. Nodes still need compatible network settings, domain IDs, message types, and Quality of Service (QoS) policies to communicate.

## 2. The basic building blocks

### Nodes

A **node** is a running participant in the ROS graph, usually focused on one responsibility: reading a sensor, estimating motion, planning a path, or controlling an actuator. C++ nodes normally use `rclcpp`; Python nodes use `rclpy`.

A node can publish or subscribe to topics, provide or call services, execute or request actions, and declare parameters. A package can install several node executables.

### Topics and messages

A **topic** is a named stream of typed messages. Publishers send messages without waiting for a particular receiver; subscriptions receive messages asynchronously. Multiple publishers and subscribers can use the same topic.

Examples often encountered in mobile robotics include:

| Data | Common interface | Example topic |
| --- | --- | --- |
| 2D laser scan | `sensor_msgs/msg/LaserScan` | `/scan` |
| Odometry estimate | `nav_msgs/msg/Odometry` | `/odom` |
| Velocity command | `geometry_msgs/msg/Twist` | `/cmd_vel` |
| Transform tree update | `tf2_msgs/msg/TFMessage` | `/tf` |

The topic name alone is not enough: publishers and subscribers must agree on the message type and have compatible QoS.

### Services and actions

| Interface | Communication pattern | Use it for |
| --- | --- | --- |
| Topic | Continuous, asynchronous stream | sensor samples, state, commands |
| Service | Short request and response | query state, reset, quick calculation |
| Action | Long-running goal with feedback, result, and cancellation | navigate to a pose, follow a trajectory |

Choose a topic when data is continuously produced or consumed. Choose a service when a short operation has one response. Choose an action when the caller needs progress or may cancel a long operation.

### Parameters

Parameters are named, typed configuration values owned by a node. Declare a parameter before reading it, and give it a useful default. Parameters are not a global parameter server shared by every node. They can be set at startup through a launch file or YAML file, and inspected or changed at runtime with `ros2 param`.

### QoS (Quality of Service)

QoS controls how data is queued and delivered. The most important policies to understand first are:

- **History and depth**: how many recent samples are kept (`KEEP_LAST`, depth N) or whether all are kept (`KEEP_ALL`, subject to resource limits).
- **Reliability**: `RELIABLE` retries delivery; `BEST_EFFORT` prioritizes timely data and may lose samples.
- **Durability**: `VOLATILE` sends new samples only to current subscribers; `TRANSIENT_LOCAL` lets a publisher retain samples for late-joining subscribers.

Sensor streams often favor fresh samples over retransmission and use a sensor-data QoS profile. Configuration or latched state may need different durability. Endpoints with incompatible QoS can fail to exchange data even when topic names and types match. Inspect actual endpoints with `ros2 topic info /topic --verbose` before changing profiles.

## 3. How a C++ node runs

A typical node follows this sequence:

1. `rclcpp::init(argc, argv)` initializes the ROS client library.
2. Construct a class derived from `rclcpp::Node`; create publishers, subscriptions, parameters, and timers.
3. `rclcpp::spin(node)` runs an executor. The executor waits for ready callbacks and invokes them.
4. `rclcpp::shutdown()` cleans up after spinning ends.

The executor makes the program event-driven: a subscription callback runs when a message arrives, a timer callback runs when its period elapses, and service/action callbacks run when requests or goal events arrive. A callback should generally avoid long blocking work because that can delay other callbacks handled by the same executor.

The lecture's example combines a `std_msgs/msg/Float64` subscription, a `std_msgs/msg/String` publisher, a timer, and a `run_period` parameter. This compact version shows the same pattern:

```cpp
#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/string.hpp"

class ExampleNode : public rclcpp::Node {
public:
  ExampleNode() : Node("example_node") {
    const auto period_ms = declare_parameter<int>("run_period_ms", 1000);

    publisher_ = create_publisher<std_msgs::msg::String>("out_topic", 10);
    subscription_ = create_subscription<std_msgs::msg::Float64>(
      "in_topic", 10,
      [this](std_msgs::msg::Float64::ConstSharedPtr msg) {
        last_value_ = msg->data;
        RCLCPP_INFO(get_logger(), "Received %.2f", last_value_);
    });

    timer_ = create_wall_timer(
      std::chrono::milliseconds(period_ms), [this] {
        std_msgs::msg::String msg;
        msg.data = "I'm alive";
        publisher_->publish(msg);
      });
  }

private:
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr subscription_;
  rclcpp::TimerBase::SharedPtr timer_;
  double last_value_{0.0};
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ExampleNode>());
  rclcpp::shutdown();
  return 0;
}
```

The integer queue depth is a shorthand QoS setting. For production or control code, choose an explicit profile based on the data's timing and loss requirements rather than assuming that one depth is right for every topic. The message types above require `std_msgs` as a package dependency and includes for `std_msgs/msg/string.hpp` and `std_msgs/msg/float64.hpp`.

## 4. Packages and workspaces

A **package** is the unit of ROS software organization. A C++ package commonly contains:

```text
my_package/
├── CMakeLists.txt        # CMake targets, dependencies, installation rules
├── package.xml           # package metadata and dependency declarations
├── include/my_package/   # public C++ headers (when needed)
├── src/                  # C++ implementation files
├── launch/               # launch descriptions (when needed)
└── config/               # YAML and other configuration (when needed)
```

A **workspace** contains one or more packages under `src/`. `colcon` discovers packages and generates `build/`, `install/`, and `log/` at the workspace root. Commit package source and required configuration, not generated output.

This computer uses:

```text
~/Projects/ros2_ws/
├── src/ros2_clion_smoke/  # package repository
├── build/                 # generated
├── install/               # generated
└── log/                   # generated
```

## 5. Build and run on this Ubuntu installation

Start a terminal and activate ROS 2. Interactive terminals may already source it through `~/.bashrc`; the explicit command is safe and makes scripts reproducible:

```bash
source /opt/ros/lyrical/setup.bash
cd ~/Projects/ros2_ws
colcon build --packages-select ros2_clion_smoke
source install/setup.bash
ros2 run ros2_clion_smoke hello_node
```

The smoke node prints a log message once per second. Stop it with `Ctrl+C`. To build every package in the workspace, omit `--packages-select ros2_clion_smoke`. `--symlink-install` is optional and useful during development:

```bash
colcon build --symlink-install
```

Create a new C++ package with `rclcpp` and `std_msgs` like this:

```bash
source /opt/ros/lyrical/setup.bash
cd ~/Projects/ros2_ws/src
ros2 pkg create --build-type ament_cmake --dependencies rclcpp std_msgs my_first_package
```

Edit its `CMakeLists.txt` to add executables and installation rules, then build from the workspace root. Declare dependencies in both `package.xml` and CMake as required by the package template and build setup. Run an installed executable with:

```bash
ros2 run my_first_package my_node
```

## 6. Launch files and configuration

Use a launch file to start multiple nodes together and supply remappings or parameters. Python launch files are common in ROS 2. A launch file belongs under `launch/` and must be installed by the package so `ros2 launch` can find it after the build.

A typical command is:

```bash
ros2 launch my_first_package robot_bringup.launch.py
```

Parameter YAML is scoped by node name, for example:

```yaml
/example_node:
  ros__parameters:
    run_period_ms: 1000
    max_speed: 0.5
```

## 7. Useful inspection commands

Use two or more terminals: one to run nodes and another to inspect the graph. Source ROS 2 in each terminal; source the workspace overlay in terminals that need your built packages.

```bash
ros2 node list
ros2 node info /example_node

ros2 topic list -t
ros2 topic type /scan
ros2 topic info /scan --verbose
ros2 topic hz /scan
ros2 topic echo /scan

ros2 service list -t
ros2 action list -t

ros2 param list
ros2 param get /example_node run_period_ms
ros2 param set /example_node run_period_ms 500
```

To publish a one-off velocity message for inspection (adapt the topic and values to the running simulator):

```bash
ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist \
  '{linear: {x: 0.1}, angular: {z: 0.0}}'
```

Treat velocity commands as actuator commands: first verify the topic, type, units, and simulator/robot state. Use low values in simulation and never publish commands to hardware unless the lab procedure explicitly permits it.

## 8. Topics, names, and mobile-robot data flow

ROS names can be relative or absolute. In a node named `/robot_a/controller`, a relative topic `cmd_vel` resolves within the node namespace, while `/cmd_vel` is global. Namespaces and remappings let the same software run multiple robots without changing source code. Always inspect the resolved topic names with `ros2 node info` and `ros2 topic list`.

A common mobile-robot pipeline looks like:

```text
LiDAR driver --/scan--> localization / obstacle processing
wheel odometry --/odom--> state estimation / controller
planner --/cmd_vel--> base controller --> wheel motors
```

`/tf` and `/tf_static` describe coordinate-frame relationships, such as `map`, `odom`, `base_link`, and a sensor frame. Frame IDs and timestamps matter: correct numeric values in the wrong frame or at the wrong time are still incorrect robot data. Confirm each message's frame and units when debugging.

## 9. Quick troubleshooting

- **`ros2: command not found`**: run `source /opt/ros/lyrical/setup.bash` and check that `/opt/ros/lyrical` exists.
- **`Package not found` after building**: from the workspace root, build it and then `source install/setup.bash` in the current terminal.
- **No nodes or topics appear**: ensure both terminals sourced the same ROS installation, use the same `ROS_DOMAIN_ID`, and check network/firewall/middleware configuration.
- **Topic exists but no messages arrive**: compare message type and QoS with `ros2 topic info /topic --verbose`; check that the publisher is running and the namespace/remapping is correct.
- **Callback does not seem to run**: confirm the node is spinning, the callback is registered, the publisher is active, and QoS is compatible.
- **Stale or surprising build**: rebuild from the workspace root and source the latest `install/setup.bash`; inspect `build/`, `install/`, and `log/` for build output.
- **CLion cannot resolve ROS headers**: use the package's local ROS-aware CMake profile/preset and verify that `/opt/ros/lyrical/setup.bash` is sourced in its environment.

## 10. How the lecture slides map to this setup

- *02 - Introduction to ROS.pdf*: ROS 2 overview and features (slides 2-3); environment and workspace setup (5-6); nodes, topics, messages, and CLI inspection (8-13); DDS and QoS (14-15); services, actions, parameters, and packages (16-20).
- *03 - An example of ROS node.pdf*: C++ package structure and manifest (2-6); `main`, node class, executor, parameters, publisher, subscriber, and timer (7-17); QoS and logging (18-21); CMake, build, run, launch, and YAML parameters (22-27).

The first deck's installation commands target Jazzy on Ubuntu 24.04. Do not copy its distro-specific apt commands into this Lyrical installation. For day-to-day source/build commands, use the local paths in this guide.

## Further reading

- [ROS 2 concepts: nodes](https://github.com/ros2/ros2_documentation/blob/rolling/source/ROS-Framework/About-Nodes.rst)
- [ROS 2 Lyrical Luth release notes](https://github.com/ros2/ros2_documentation/blob/rolling/source/Releases/Release-Lyrical-Luth.rst)
- [ROS 2 topics, services, and actions](https://github.com/ros2/ros2_documentation/blob/rolling/source/ROS-Framework/Interfaces-Topics-Services.rst)
- [ROS 2 QoS settings](https://github.com/ros2/ros2_documentation/blob/rolling/source/ROS-Framework/interfaces/topics/About-Quality-of-Service-Settings.rst)
- [ROS 2 C++ package tutorial](https://docs.ros.org/en/rolling/Tutorials/Beginner-Client-Libraries/Creating-Your-First-ROS2-Package.html)
- [ROS 2 C++ publisher/subscriber tutorial](https://docs.ros.org/en/rolling/Tutorials/Beginner-Client-Libraries/Writing-A-Simple-Cpp-Publisher-And-Subscriber.html)
