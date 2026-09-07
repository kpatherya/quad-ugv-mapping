#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <cv_bridge/cv_bridge.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <image_geometry/pinhole_camera_model.h>

class DepthToPointCloudConverter : public rclcpp::Node
{
private:
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr depth_sub_;
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_sub_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pointcloud_pub_;

    image_geometry::PinholeCameraModel camera_model_;
    bool camera_info_received_ = false;

    // parameters
    double max_depth_;
    double min_depth_;
    int skip_pixels_;

public:
    DepthToPointCloudConverter() : Node("depth_pointcloud_converter")
    {
        // parameters
        this->declare_parameter("max_depth", 5.0);
        this->declare_parameter("min_depth", 0.3);
        this->declare_parameter("skip_pixels", 2);  // skip every N pixels for performance

        max_depth_ = this->get_parameter("max_depth").as_double();
        min_depth_ = this->get_parameter("min_depth").as_double();
        skip_pixels_ = this->get_parameter("skip_pixels").as_int();

        // subscribers
        depth_sub_ = this->create_subscription<sensor_msgs::msg::Image>("/camera/camera/depth/image_rect_raw", 10, std::bind(&DepthToPointCloudConverter::depthCallback, this, std::placeholders::_1));

        camera_info_sub_ = this->create_subscription<sensor_msgs::msg::CameraInfo>("/camera/camera/depth/camera_info", 10, std::bind(&DepthToPointCloudConverter::cameraInfoCallback, this, std::placeholders::_1));

        // publisher
        pointcloud_pub_ = this->create_publisher<sensor_msgs::msg::PointCloud2>("/depth_pointcloud", 10);

        RCLCPP_INFO(this->get_logger(), "Depth to PointCloud converter initialized");
    }

    void cameraInfoCallback(const sensor_msgs::msg::CameraInfo::SharedPtr msg)
    {
        if (!camera_info_received_) {
            camera_model_.fromCameraInfo(msg);
            camera_info_received_ = true;
            RCLCPP_INFO(this->get_logger(), "Camera info received");
        }
    }

    void depthCallback(const sensor_msgs::msg::Image::SharedPtr depth_msg)
    {
        if (!camera_info_received_) {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Camera info not received yet");
            return;
        }

        try {
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(depth_msg, sensor_msgs::image_encodings::TYPE_16UC1);
            cv::Mat depth_image = cv_ptr->image;

            pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
            cloud->header.frame_id = depth_msg->header.frame_id;
            cloud->header.stamp = depth_msg->header.stamp.sec * 1000000LL + depth_msg->header.stamp.nanosec / 1000;

            // convert depth image to point cloud
            for (int v = 0; v < depth_image.rows; v += skip_pixels_) {
                for (int u = 0; u < depth_image.cols; u += skip_pixels_) {
                    uint16_t depth_value = depth_image.at<uint16_t>(v, u);

                    if (depth_value == 0) continue;   // invalid depth

                    double depth = depth_value * 0.001;    // convert mm to meters

                    if (depth < min_depth_ || depth > max_depth_) continue;

                    // project to 3D using camera model
                    cv::Point3d ray = camera_model_.projectPixelTo3dRay(cv::Point2d(u, v));

                    pcl::PointXYZ point;
                    point.x = ray.x * depth;
                    point.y = ray.y * depth;
                    point.z = ray.z * depth;

                    cloud->points.push_back(point);
                }
            }

            cloud->width = cloud->points.size();
            cloud->height = 1;
            cloud->is_dense = false;

            // convert to ROS message and publish
            sensor_msgs::msg::PointCloud2 output;
            pcl::toROSMsg(*cloud, output);
            output.header = depth_msg->header;

            pointcloud_pub_->publish(output);
        
        } catch (cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "CV Bridge error: %s", e.what());
        }
    }
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DepthToPointCloudConverter>());
    rclcpp::shutdown();
    return 0;
}