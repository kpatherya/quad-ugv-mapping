#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/camera_info.hpp>

class CameraInfoPublisher : public rclcpp::Node
{
private:
    rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr color_info_pub_;
    rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr depth_info_pub_;
    rclcpp::TimerBase::SharedPtr timer_;

public:
    CameraInfoPublisher() : Node("camera_info_publisher")
    {
        // Publishers for both cameras
        color_info_pub_ = this->create_publisher<sensor_msgs::msg::CameraInfo>(
            "/camera/camera/color/camera_info", 10);
            
        depth_info_pub_ = this->create_publisher<sensor_msgs::msg::CameraInfo>(
            "/camera/camera/depth/camera_info", 10);
        
        // Timer for publishing
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(33), // ~30Hz
            std::bind(&CameraInfoPublisher::publishCameraInfo, this));
        
        RCLCPP_INFO(this->get_logger(), "Camera info publisher initialized");
    }

private:
    void publishCameraInfo()
    {
        auto now = this->now();
        
        // Create and publish color camera info based on YAML values
        auto color_info = createColorCameraInfo();
        color_info.header.stamp = now;
        color_info_pub_->publish(color_info);
        
        // Create and publish depth camera info (based on color with adjustments)
        auto depth_info = createDepthCameraInfo();
        depth_info.header.stamp = now;
        depth_info_pub_->publish(depth_info);
    }
    
    sensor_msgs::msg::CameraInfo createColorCameraInfo()
    {
        sensor_msgs::msg::CameraInfo info;
        
        // From YAML file
        info.header.frame_id = "camera_color_optical_frame";
        info.height = 480;
        info.width = 640;
        
        // Distortion model from YAML
        info.distortion_model = "plumb_bob";
        
        // Distortion parameters (k1, k2, p1, p2, k3)
        info.d = {9.2615504465028850e-02, -1.8082438825995681e-01, 
                 -6.5484100374765971e-04, -3.5829351558557421e-04, 0.0};
        
        // Camera matrix (K) [fx, 0, cx; 0, fy, cy; 0, 0, 1]
        info.k[0] = 6.0970550296798035e+02; // fx
        info.k[2] = 3.1916667152289227e+02; // cx
        info.k[4] = 6.0909579671294716e+02; // fy
        info.k[5] = 2.3558360480225772e+02; // cy
        info.k[8] = 1.0;
        
        // Rectification matrix (R) - identity for pinhole
        info.r[0] = 1.0;
        info.r[4] = 1.0;
        info.r[8] = 1.0;
        
        // Projection matrix (P) [fx, 0, cx, 0; 0, fy, cy, 0; 0, 0, 1, 0]
        info.p[0] = info.k[0]; // fx
        info.p[2] = info.k[2]; // cx
        info.p[5] = info.k[4]; // fy
        info.p[6] = info.k[5]; // cy
        info.p[10] = 1.0;
        
        return info;
    }
    
    sensor_msgs::msg::CameraInfo createDepthCameraInfo()
    {
        // For the D435i, depth camera has different intrinsics
        // If you have them, use the exact values; otherwise we'll
        // use the color values with typical D435i depth adjustments
        
        sensor_msgs::msg::CameraInfo info;
        info.header.frame_id = "camera_depth_optical_frame";
        info.height = 480;
        info.width = 640;
        info.distortion_model = "plumb_bob";
        
        // D435i typical depth has minimal distortion
        info.d = {0.0, 0.0, 0.0, 0.0, 0.0};
        
        // Typical D435i depth camera values (slightly different from color)
        // If you have actual calibration, replace these
        info.k[0] = 383.285; // fx
        info.k[2] = 320.0;   // cx
        info.k[4] = 383.285; // fy
        info.k[5] = 240.0;   // cy
        info.k[8] = 1.0;
        
        // Rectification matrix - identity
        info.r[0] = 1.0;
        info.r[4] = 1.0;
        info.r[8] = 1.0;
        
        // Projection matrix
        info.p[0] = info.k[0]; // fx
        info.p[2] = info.k[2]; // cx
        info.p[5] = info.k[4]; // fy
        info.p[6] = info.k[5]; // cy
        info.p[10] = 1.0;
        
        return info;
    }
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CameraInfoPublisher>());
    rclcpp::shutdown();
    return 0;
}