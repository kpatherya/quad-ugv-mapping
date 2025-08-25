from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo
from ament_index_python.packages import get_package_share_directory
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node

def generate_launch_description():
    # Locate the package
    config_pkg_path = get_package_share_directory('config_pkg')

    # Declare launch argument for config path
    declare_config_arg = DeclareLaunchArgument(
        'config_path',
        default_value=PathJoinSubstitution([
            config_pkg_path, 'config/euroc/euroc_config.yaml'
        ]),
        description='Path to the VINS-MONO config YAML'
    )

    # Use LaunchConfiguration to pick up CLI override or default
    config_path = LaunchConfiguration('config_path')
    vins_path = PathJoinSubstitution([config_pkg_path, 'config/../'])
    support_path = PathJoinSubstitution([config_pkg_path, 'support_files'])

    # Define the vins_estimator node
    vins_estimator_node = Node(
        package='vins_estimator',
        executable='vins_estimator',
        name='vins_estimator',
        namespace='vins_estimator',
        output='screen',
        parameters=[{
            'config_file': config_path,
            'vins_folder': vins_path
        }]
    )

    # Define the pose_graph node
    pose_graph_node = Node(
        package='pose_graph',
        executable='pose_graph',
        name='pose_graph',
        namespace='pose_graph',
        output='screen',
        parameters=[{
            'config_file': config_path,
            'support_file': support_path,
            'visualization_shift_x': 0,
            'visualization_shift_y': 0,
            'skip_cnt': 0,
            'skip_dis': 0.0
        }]
    )

    # adding the point cloud converter node
    pointcloud_converter_node = Node(
        package='point_cloud_converter',
        executable='point_cloud_converter_node',
        name='pointcloud_converter',
        output='screen',
        parameters=[{
            'points_in': '/vins_estimator/point_cloud',
            'points2_out': '/point_cloud_converter/output',
            'target_frame': 'map'  # set frame_id to match octomap server
        }]
    )

    # TF publishers for camera frames
    tf_static_publisher_world_map = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_transform_publisher_world_map',
        arguments=['0', '0', '0', '0', '0', '0', 'world', 'map']
    )

    # camera base to map transform (adjust values based on camera mounting)
    tf_static_publisher_camera_base = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_transform_publisher_camera_base',
        arguments=['0', '0', '0', '0', '0', '0', 'map', 'camera_link']
    )

    # camera depth optical frame
    tf_static_publisher_depth_optical = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_transform_publisher_depth_optical',
        arguments=['0', '0', '0', '-1.5708', '0', '-1.5708', 'camera_link', 'camera_depth_optical_frame']
    )

    # dense depth-based point cloud converter
    depth_pointcloud_converter_node = Node(
        package='vins_estimator',
        executable='depth_pointcloud_converter',
        name='depth_pointcloud_converter',
        output='screen',
        parameters=[{
            'max_depth': 5.0,       # match octomap range
            'min_depth': 0.3,       # avoid noise close to camera
            'skip_pixels': 2        # downsample for performance
        }],
        remappings=[
            ('depth_image', '/camera/camera/depth/image_rect_raw'),
            ('camera_info', '/camera/camera/depth/camera_info'),
            ('pointcloud_out', '/dense_pointcloud')     # map to what octomap expects
        ]
    )

    # synthetic camera info publisher for depth camera
    camera_info_publisher_node = Node(
        package='vins_estimator',
        executable='camera_info_publisher',
        name='camera_info_publisher',
        output='screen'
    )

    # octomap server node
    octomap_server_node = Node(
        package='octomap_server',
        executable='octomap_server_node',
        name='octomap_server',
        output='screen',
        parameters=[{
            'resolution': 0.02,           # Decrease for higher detail (was 0.05)
            'frame_id': 'map',
            'sensor_model.max_range': 5.0, # Increase range to capture more data
            'sensor_model.hit': 0.7,
            'sensor_model.miss': 0.3,     # Slightly lower to reduce noise
            # 'occupancy_min_z': -0.2,      # Limit to actual floor
            # 'occupancy_max_z': 2.5,       # Limit to ceiling height
            'height_map': True,            # Better visualization
        }],
        remappings=[
            ('cloud_in', '/depth_pointcloud')   # use direct output from depth converter
        ]
    )

    # BETTER TO USE ONLY ONE RVIZ WINDOW
    # rviz node with custom config
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', PathJoinSubstitution([
            config_pkg_path, 'config/trajectory_and_map.rviz'
        ])]
    )

    return LaunchDescription([
        # Log the paths
        LogInfo(msg=['[vins estimator launch] config path: ', config_path]),
        LogInfo(msg=['[vins estimator launch] vins path: ', vins_path]),
        LogInfo(msg=['[vins estimator launch] support path: ', support_path]),
        # Include the declared argument
        declare_config_arg,
        tf_static_publisher_world_map,
        tf_static_publisher_camera_base,
        tf_static_publisher_depth_optical,
        camera_info_publisher_node,
        vins_estimator_node,
        pose_graph_node,
        depth_pointcloud_converter_node,
        # pointcloud_converter_node,
        octomap_server_node,
        rviz_node
    ])