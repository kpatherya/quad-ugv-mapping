#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <px4_msgs/msg/vehicle_odometry.hpp>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>

using std::placeholders::_1;

class VIOBridgeNode : public rclcpp::Node
{
public:
    VIOBridgeNode()
    : Node("vio_to_px4_bridge"),
      last_pub_time_(this->now()),
      pub_interval_(0, 0)
    {
        // parameters: camera pitch (deg) and publish rate (Hz)
        this->declare_parameter<double>("camera_pitch_deg", 10.0);
        this->declare_parameter<double>("vio_pub_rate",      20.0);

        double cam_pitch_deg = this->get_parameter("camera_pitch_deg").as_double();
        double cam_pitch_rad = cam_pitch_deg * M_PI / 180.0;
        double vio_rate      = this->get_parameter("vio_pub_rate").as_double();
        uint32_t interval_ns = static_cast<uint32_t>(1e9 / vio_rate);
        pub_interval_ = rclcpp::Duration(0, interval_ns);

        // build camera→body (IMU) quaternion: rotY(-pitch)
        q_cam2body_ = Eigen::Quaterniond(
          Eigen::AngleAxisd(-cam_pitch_rad, Eigen::Vector3d::UnitY())
        );

        // build NWU→FRD quaternion: roll 180°
        q_nwu2frd_ = Eigen::Quaterniond(
          Eigen::AngleAxisd(M_PI, Eigen::Vector3d::UnitX())
        );

        // subscribe to VINS-Mono odometry (ENU)
        sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/vins_estimator/odometry", 10,
            std::bind(&VIOBridgeNode::odom_callback, this, _1)
        );

        // publish PX4 VehicleOdometry in FRD
        pub_ = this->create_publisher<px4_msgs::msg::VehicleOdometry>(
            "/fmu/in/vehicle_visual_odometry", 10
        );
    }

private:
    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        // enforce publish rate
        rclcpp::Time now = this->now();
        if ((now - last_pub_time_) < pub_interval_) {
            return;
        }
        last_pub_time_ = now;

        px4_msgs::msg::VehicleOdometry odom_msg;

        //
        // --- TIMESTAMPS (µs) ---
        //
        // when PX4 receives it
        odom_msg.timestamp = now.nanoseconds() / 1000;

        // when VIO measured it
        rclcpp::Time sample_time(msg->header.stamp);
        odom_msg.timestamp_sample = sample_time.nanoseconds() / 1000;

        //
        // --- POSE (ENU → FRD) ---
        //
        odom_msg.pose_frame = px4_msgs::msg::VehicleOdometry::POSE_FRAME_FRD;
        odom_msg.position[0] =  msg->pose.pose.position.x;   // forward
        odom_msg.position[1] = -msg->pose.pose.position.y;   // right
        odom_msg.position[2] = -msg->pose.pose.position.z;   // down

        // orientation: camera→body→NWU→FRD
        Eigen::Quaterniond q_vio(
            msg->pose.pose.orientation.w,
            msg->pose.pose.orientation.x,
            msg->pose.pose.orientation.y,
            msg->pose.pose.orientation.z
        );
        Eigen::Quaterniond q_body = q_cam2body_ * q_vio;
        Eigen::Quaterniond q_frd  = q_nwu2frd_  * q_body * q_nwu2frd_.inverse();
        odom_msg.q[0] = q_frd.w();
        odom_msg.q[1] = q_frd.x();
        odom_msg.q[2] = q_frd.y();
        odom_msg.q[3] = q_frd.z();

        //
        // --- VELOCITY (ENU → FRD body) ---
        //
        odom_msg.velocity_frame = px4_msgs::msg::VehicleOdometry::VELOCITY_FRAME_BODY_FRD;
        odom_msg.velocity[0] =  msg->twist.twist.linear.x;
        odom_msg.velocity[1] = -msg->twist.twist.linear.y;
        odom_msg.velocity[2] = -msg->twist.twist.linear.z;

        // angular rates (body FRD)
        odom_msg.angular_velocity[0] =  msg->twist.twist.angular.x;
        odom_msg.angular_velocity[1] = -msg->twist.twist.angular.y;
        odom_msg.angular_velocity[2] = -msg->twist.twist.angular.z;

        //
        // --- COVARIANCES ---
        //
        constexpr double POS_VAR = 1.0, ORI_VAR = 0.1, VEL_VAR = 1.0;
        // position
        odom_msg.position_variance[0]  = POS_VAR;
        odom_msg.position_variance[7]  = POS_VAR;
        odom_msg.position_variance[14] = POS_VAR;
        // orientation
        odom_msg.orientation_variance[0]  = ORI_VAR;
        odom_msg.orientation_variance[7]  = ORI_VAR;
        odom_msg.orientation_variance[14] = ORI_VAR;
        // velocity
        odom_msg.velocity_variance[0]  = VEL_VAR;
        odom_msg.velocity_variance[7]  = VEL_VAR;
        odom_msg.velocity_variance[14] = VEL_VAR;
        // leave angular‐velocity covariance zero

        odom_msg.reset_counter = 0;
        odom_msg.quality       = 0;

        pub_->publish(odom_msg);
    }

    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr    sub_;
    rclcpp::Publisher<px4_msgs::msg::VehicleOdometry>::SharedPtr pub_;
    rclcpp::Time          last_pub_time_;
    rclcpp::Duration      pub_interval_;
    Eigen::Quaterniond    q_cam2body_;
    Eigen::Quaterniond    q_nwu2frd_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VIOBridgeNode>());
    rclcpp::shutdown();
    return 0;
}
