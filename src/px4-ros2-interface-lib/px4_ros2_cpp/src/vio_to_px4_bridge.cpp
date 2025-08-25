#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <px4_msgs/msg/vehicle_odometry.hpp>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>

// This script is for Open-VINS
// Example usage:
//   ros2 run px4_ros2_cpp vio_to_px4_bridge 
//     --ros-args -p vio_pub_rate:=20.0 -p camera_pitch_deg:=10.0

using std::placeholders::_1;

class VIOBridgeNode : public rclcpp::Node
{
public:
    VIOBridgeNode()
    : Node("vio_to_px4_bridge"),
      last_pub_time_(this->now()),
      pub_interval_(0, 0)
    {
        this->declare_parameter<double>("camera_pitch_deg", 10.0);
        this->declare_parameter<double>("vio_pub_rate",      20.0);

        double cam_pitch_deg = this->get_parameter("camera_pitch_deg").as_double();
        double cam_pitch_rad = cam_pitch_deg * M_PI / 180.0;
        double vio_rate      = this->get_parameter("vio_pub_rate").as_double();
        pub_interval_ = rclcpp::Duration(0, static_cast<uint32_t>(1e9 / vio_rate));

        // ENU → NED: rotate -90° about Z, then 180° about X
        Eigen::AngleAxisd rot_z(-M_PI_2, Eigen::Vector3d::UnitZ());
        Eigen::AngleAxisd rot_x( M_PI,    Eigen::Vector3d::UnitX());
        q_enu2ned_ = Eigen::Quaterniond(rot_x * rot_z);

        // Camera → body: rotate by -pitch about camera Y
        q_cam2body_ = Eigen::Quaterniond(
          Eigen::AngleAxisd(-cam_pitch_rad, Eigen::Vector3d::UnitY())
        );

        sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/ov_msckf/odomimu", 10,
            std::bind(&VIOBridgeNode::odom_callback, this, _1)
        );
        pub_ = this->create_publisher<px4_msgs::msg::VehicleOdometry>(
            "/fmu/in/vehicle_visual_odometry", 10
        );
    }

private:
    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        rclcpp::Time now = this->now();
        if ((now - last_pub_time_) < pub_interval_) {
            return;
        }
        last_pub_time_ = now;

        px4_msgs::msg::VehicleOdometry odom_msg;

        // timestamps in microseconds
        odom_msg.timestamp        = now.nanoseconds() / 1000;
        odom_msg.timestamp_sample = odom_msg.timestamp;

        // Position ENU → NED
        odom_msg.pose_frame  = px4_msgs::msg::VehicleOdometry::POSE_FRAME_NED;
        odom_msg.position[0] =  msg->pose.pose.position.y;
        odom_msg.position[1] =  msg->pose.pose.position.x;
        odom_msg.position[2] = -msg->pose.pose.position.z;

        // Orientation ENU → body → NED
        Eigen::Quaterniond q_vio(
            msg->pose.pose.orientation.w,
            msg->pose.pose.orientation.x,
            msg->pose.pose.orientation.y,
            msg->pose.pose.orientation.z
        );
        Eigen::Quaterniond q_body = q_cam2body_ * q_vio;
        Eigen::Quaterniond q_ned  = q_enu2ned_   * q_body;
        odom_msg.q[0] = q_ned.w();
        odom_msg.q[1] = q_ned.x();
        odom_msg.q[2] = q_ned.y();
        odom_msg.q[3] = q_ned.z();

        // Velocity ENU → NED
        odom_msg.velocity_frame = px4_msgs::msg::VehicleOdometry::VELOCITY_FRAME_NED;
        odom_msg.velocity[0]    =  msg->twist.twist.linear.y;
        odom_msg.velocity[1]    =  msg->twist.twist.linear.x;
        odom_msg.velocity[2]    = -msg->twist.twist.linear.z;

        // Angular rates
        odom_msg.angular_velocity[0] =  msg->twist.twist.angular.y;
        odom_msg.angular_velocity[1] =  msg->twist.twist.angular.x;
        odom_msg.angular_velocity[2] = -msg->twist.twist.angular.z;

        // --- Copy covariance straight from Open-VINS odomimu ---
        // Pose covariance (row-major 6×6): take [0], [7], [14]
        odom_msg.position_variance[0]    = msg->pose.covariance[0];
        odom_msg.position_variance[1]    = msg->pose.covariance[7];
        odom_msg.position_variance[2]    = msg->pose.covariance[14];

        // Orientation covariance (row-major 6×6): take [21], [28], [35]
        odom_msg.orientation_variance[0] = msg->pose.covariance[21];
        odom_msg.orientation_variance[1] = msg->pose.covariance[28];
        odom_msg.orientation_variance[2] = msg->pose.covariance[35];

        // Velocity covariance (row-major 6×6): take [0], [7], [14]
        odom_msg.velocity_variance[0]    = msg->twist.covariance[0];
        odom_msg.velocity_variance[1]    = msg->twist.covariance[7];
        odom_msg.velocity_variance[2]    = msg->twist.covariance[14];

        odom_msg.reset_counter = 0;
        odom_msg.quality       = 0;

        pub_->publish(odom_msg);
    }

    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr sub_;
    rclcpp::Publisher<px4_msgs::msg::VehicleOdometry>::SharedPtr pub_;
    rclcpp::Time     last_pub_time_;
    rclcpp::Duration pub_interval_;
    Eigen::Quaterniond q_enu2ned_;
    Eigen::Quaterniond q_cam2body_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VIOBridgeNode>());
    rclcpp::shutdown();
    return 0;
}
