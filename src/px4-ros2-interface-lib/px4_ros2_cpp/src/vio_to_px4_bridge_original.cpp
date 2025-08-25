#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <px4_msgs/msg/vehicle_odometry.hpp>

#include <Eigen/Core>
#include <Eigen/Geometry>

using std::placeholders::_1;

class VIOBridgeNode : public rclcpp::Node
{
public:
    VIOBridgeNode()
    : Node("vio_to_px4_bridge")
    {
        sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/ov_msckf/odomimu", 10,
            std::bind(&VIOBridgeNode::odom_callback, this, _1));

        pub_ = this->create_publisher<px4_msgs::msg::VehicleOdometry>(
            "/fmu/in/vehicle_visual_odometry", 10);
    }

private:
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr sub_;
    rclcpp::Publisher<px4_msgs::msg::VehicleOdometry>::SharedPtr pub_;

    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        px4_msgs::msg::VehicleOdometry odom_msg;

        odom_msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
        odom_msg.timestamp_sample = odom_msg.timestamp;

        odom_msg.pose_frame = px4_msgs::msg::VehicleOdometry::POSE_FRAME_NED;

        odom_msg.position[0] = msg->pose.pose.position.y;
        odom_msg.position[1] = msg->pose.pose.position.x;
        odom_msg.position[2] = -msg->pose.pose.position.z;

        Eigen::Quaterniond q_enu(
            msg->pose.pose.orientation.w,
            msg->pose.pose.orientation.x,
            msg->pose.pose.orientation.y,
            msg->pose.pose.orientation.z);

        Eigen::Quaterniond q_rot;
        q_rot = Eigen::Quaterniond(Eigen::AngleAxisd(M_PI, Eigen::Vector3d::UnitZ()));
        Eigen::Quaterniond q_ned = q_rot * q_enu;

        odom_msg.q[0] = q_ned.w();
        odom_msg.q[1] = q_ned.x();
        odom_msg.q[2] = q_ned.y();
        odom_msg.q[3] = q_ned.z();

        odom_msg.velocity_frame = px4_msgs::msg::VehicleOdometry::VELOCITY_FRAME_NED;
        odom_msg.velocity[0] = msg->twist.twist.linear.y;
        odom_msg.velocity[1] = msg->twist.twist.linear.x;
        odom_msg.velocity[2] = -msg->twist.twist.linear.z;

        odom_msg.angular_velocity[0] = msg->twist.twist.angular.y;
        odom_msg.angular_velocity[1] = msg->twist.twist.angular.x;
        odom_msg.angular_velocity[2] = -msg->twist.twist.angular.z;

        odom_msg.position_variance[0] = msg->pose.covariance[0];
        odom_msg.position_variance[1] = msg->pose.covariance[7];
        odom_msg.position_variance[2] = msg->pose.covariance[14];

        odom_msg.orientation_variance[0] = msg->pose.covariance[21];
        odom_msg.orientation_variance[1] = msg->pose.covariance[28];
        odom_msg.orientation_variance[2] = msg->pose.covariance[35];

        odom_msg.velocity_variance[0] = msg->twist.covariance[0];
        odom_msg.velocity_variance[1] = msg->twist.covariance[7];
        odom_msg.velocity_variance[2] = msg->twist.covariance[14];

        odom_msg.reset_counter = 0;
        odom_msg.quality = 0;

        pub_->publish(odom_msg);
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VIOBridgeNode>());
    rclcpp::shutdown();
    return 0;
}
