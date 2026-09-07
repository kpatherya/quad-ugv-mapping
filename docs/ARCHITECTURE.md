# Architecture

## Scope

This workspace combines independently developed ROS packages into an
experimental visual-inertial mapping pipeline. Its reusable contribution is the
integration boundary: launch composition, sensor-message conversion, TF-frame
alignment, dataset configuration, and a documented validation workflow.

It is not a decentralized multi-robot coordinator and does not contain a
semantic ICP implementation.

## Source boundaries

| Directory | Role | Change policy |
| --- | --- | --- |
| `src/toolbox` | Small pipeline utilities | Add tests and stable ROS parameters |
| `src/integrations` | Adapted upstream systems plus project wiring | Keep upstream attribution and document local changes |
| `src/vendor` | Imported upstream snapshots | Avoid unrelated edits; prefer upstream fixes |

Colcon searches recursively below `src`, so the organizational directories do
not alter ROS package names or runtime lookup through the ament index.

## Reference data flow

1. `feature_tracker` consumes monocular images and publishes tracked features.
2. `vins_estimator` fuses those features with IMU measurements and publishes
   odometry, paths, transforms, and sparse landmarks.
3. `pose_graph` adds loop-closure constraints and publishes corrected paths.
4. `depth_pointcloud_converter` projects registered depth images using camera
   intrinsics.
5. `point_cloud_converter` adapts legacy sparse `PointCloud` output to
   `PointCloud2` where required.
6. `octomap_server` integrates a selected `PointCloud2` stream in the `map`
   frame.
7. RViz displays trajectories, point clouds, TF, and occupancy output.

## Runtime contracts

The default EuRoC launch is
`src/integrations/vins_mono_ros2/vins_estimator/launch/euroc.launch.py`.
Treat its values as examples, not calibration defaults for every robot.

| Contract | Default/example | Required check |
| --- | --- | --- |
| VINS world frame | `world` | Confirm estimator output frame |
| Mapping frame | `map` | Confirm one authoritative `world -> map` transform |
| Camera base frame | `camera_link` | Replace identity transform with measured extrinsics |
| Depth optical frame | `camera_depth_optical_frame` | Verify REP-103 optical orientation |
| VINS sparse cloud | `/vins_estimator/point_cloud` | Confirm message rate and timestamp source |
| Depth image | `/camera/camera/depth/image_rect_raw` | Remap for the installed RealSense driver |
| Camera info | `/camera/camera/depth/camera_info` | Ensure synchronized calibration data |
| OctoMap input | `/depth_pointcloud` | Confirm a publisher exists before recording results |

Use `ros2 topic info`, `ros2 topic hz`, `ros2 run tf2_tools view_frames`, and
RViz TF display before interpreting a map. An identity static transform is not a
substitute for calibrated sensor extrinsics.

## Reuse guidance

- Put robot-specific calibration in a new directory below
  `config_pkg/config`; do not overwrite the EuRoC reference configuration.
- Expose topic names and frame IDs as launch arguments when extending the
  pipeline.
- Keep datasets, bags, and generated maps outside Git.
- Record the repository commit, parameter files, ROS distribution, sensor
  firmware, and dataset checksum for every experiment.
- Submit fixes to vendored projects upstream when they are not specific to this
  integration.

## Known limitations

- ROS 2 Foxy and Ubuntu 20.04 are end-of-life.
- The workspace includes historical source snapshots without recorded upstream
  commit IDs.
- No automated accuracy threshold is currently committed for EuRoC output.
- Camera and depth topic defaults reflect one lab setup.
- Static transforms in example launch files require hardware calibration.
- Hardware and graphical RViz execution cannot be validated in headless CI.