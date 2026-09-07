#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/point_cloud_conversion.hpp>

class PointCloudConverter : public rclcpp::Node
{
private:
    rclcpp::Subscription<sensor_msgs::msg::PointCloud>::SharedPtr sub_points_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub_points2_;

public:
    PointCloudConverter() : Node("point_cloud_converter")
    {
        // Declare parameters
        this->declare_parameter("points_in", "/vins_estimator/point_cloud");
        this->declare_parameter("points2_out", "/point_cloud_converter/output");

        // Get parameter values
        std::string points_in = this->get_parameter("points_in").as_string();
        std::string points2_out = this->get_parameter("points2_out").as_string();

        // Create subscriber
        sub_points_ = this->create_subscription<sensor_msgs::msg::PointCloud>(
            points_in, 10, std::bind(&PointCloudConverter::cloud_cb_points, this, std::placeholders::_1));
        
        // Create publisher
        pub_points2_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(points2_out, 10);

        RCLCPP_INFO(this->get_logger(), "PointCloudConverter initialized");
        RCLCPP_INFO(this->get_logger(), "Converting PointCloud (%s) to PointCloud2 (%s)", 
                    points_in.c_str(), points2_out.c_str());
    }

    void cloud_cb_points(const sensor_msgs::msg::PointCloud::SharedPtr msg)
    {
        sensor_msgs::msg::PointCloud2 output;
        
        // Convert PointCloud to PointCloud2
        if (!sensor_msgs::convertPointCloudToPointCloud2(*msg, output)) {
            RCLCPP_ERROR(this->get_logger(), "Conversion from PointCloud to PointCloud2 failed!");
            return;
        }
        output.header = msg->header;
        
        RCLCPP_DEBUG(this->get_logger(), "Converting PointCloud with %d points to PointCloud2", 
                     (int)msg->points.size());
        
        pub_points2_->publish(output);
    }
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PointCloudConverter>());
    rclcpp::shutdown();
    return 0;
}