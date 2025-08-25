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
        description='Path to the feature tracker config YAML'
    )

    # Use LaunchConfiguration to pick up CLI override or default
    config_path = LaunchConfiguration('config_path')
    vins_path = PathJoinSubstitution([config_pkg_path, 'config/../'])

    # Define the feature_tracker node
    feature_tracker_node = Node(
        package='feature_tracker',
        executable='feature_tracker',
        name='feature_tracker',
        namespace='feature_tracker',
        output='screen',
        parameters=[{
            'config_file': config_path,
            'vins_folder': vins_path
        }]
    )

    # Define the rviz2 node
    # rviz_config_path = PathJoinSubstitution([
    #     config_pkg_path, 'config/vins_octomap_rviz.rviz'
    # ])
    # rviz_node = Node(
    #     package='rviz2',
    #     executable='rviz2',
    #     name='rviz2',
    #     arguments=['-d', rviz_config_path],
    #     output='screen'
    # )

    return LaunchDescription([
        # Log the config path
        LogInfo(msg=['[feature tracker launch] config path: ', config_path]),
        # Include the declared argument
        declare_config_arg,
        feature_tracker_node
        # rviz_node
    ])