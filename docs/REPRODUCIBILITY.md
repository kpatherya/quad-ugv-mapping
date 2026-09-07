# Reproducibility Guide

## Reproduction levels

Use the narrowest level that supports your claim.

| Level | Goal | Hardware required |
| --- | --- | --- |
| A | Validate repository structure and metadata | No |
| B | Compile the selected ROS 2 packages | No, Linux amd64 recommended |
| C | Reproduce VIO on EuRoC | No |
| D | Reproduce room-scale depth mapping | RealSense camera or recorded bag |

## Reference environment

- Ubuntu 20.04 amd64
- ROS 2 Foxy desktop
- OpenCV 4.2
- Eigen 3.3.7
- Ceres Solver 1.14
- Python 3
- `colcon`, `rosdep`, and `vcstool`

Foxy is end-of-life. The Dockerfile captures the historical environment but
cannot guarantee continued availability of upstream apt repositories or base
images. Record the built image digest alongside experimental results.

## Level A: repository validation

```bash
python3 tools/validate_repository.py
```

This checks XML readability, unique ROS package names, expected ownership
directories, local Markdown links, and unresolved license placeholders in the
project-facing integration and toolbox directories. It does not compile C++.

## Level B: build

```bash
source /opt/ros/foxy/setup.bash
rosdep update
rosdep install --from-paths src --ignore-src --rosdistro foxy -r -y
./tools/build_workspace.sh mapping
source install/setup.bash
```

For a narrower smoke test, use `./tools/build_workspace.sh converter`. Use
`./tools/build_workspace.sh all` only when developing every vendored subsystem;
the full snapshot spans packages with different hardware and middleware needs.

Record:

```bash
git rev-parse HEAD
printenv ROS_DISTRO
colcon list --names-only | sort
dpkg-query -W > results/dpkg-packages.txt
```

## Level C: EuRoC VIO

1. Download an official EuRoC MAV sequence from ETH Zurich.
2. Record the archive checksum with `sha256sum`.
3. Convert the ROS 1 bag to ROS 2 using a pinned `rosbags` release.
4. Build the `vio` profile and source `install/setup.bash`.
5. Start the feature tracker and estimator.
6. Play the converted bag with simulated time if the selected nodes support it.
7. Start `benchmark_publisher` for the matching sequence.
8. Save the estimated and reference trajectories plus console logs.

Example launch sequence:

```bash
ros2 launch feature_tracker vins_feature_tracker.launch.py
ros2 launch vins_estimator euroc.launch.py \
  config_path:=/absolute/path/to/euroc_config.yaml
ros2 launch benchmark_publisher benchmark_publisher.launch.py \
  sequence_name:=MH_01_easy
ros2 bag play /absolute/path/to/MH_01_easy
```

Do not report an accuracy number until the trajectory coordinate frames, time
alignment, and evaluation command are recorded. A visual RViz match is a smoke
test, not a quantitative benchmark.

## Level D: RealSense and OctoMap

Before launching:

1. Record camera model, serial, firmware, stream resolution, and frame rate.
2. Store camera intrinsics and measured camera-to-body extrinsics.
3. Remap image, depth, camera-info, and IMU topics to the active driver.
4. Confirm exactly one connected TF path from sensor frames to `map`.
5. Confirm `octomap_server` has a live `PointCloud2` publisher on `cloud_in`.
6. Record the input bag and parameter files used for each map.

Minimum runtime evidence:

```bash
ros2 node list
ros2 topic list -t
ros2 topic hz /depth_pointcloud
ros2 run tf2_tools view_frames
ros2 param dump /octomap_server > results/octomap-params.yaml
```

For repeatability, replay the same bag at least three times and report whether
loop closures, final trajectory, and occupied volume are stable. Keep raw data
outside Git and publish it separately with checksums and access terms.

## Results manifest

Every published result should include:

- repository commit SHA and dirty/clean state;
- container image digest or host dependency inventory;
- dataset URL, sequence name, and SHA-256 checksum;
- launch command and parameter files;
- TF tree and topic graph;
- output trajectory/map artifact checksum;
- evaluation command and metric definition; and
- known deviations from the reference environment.

The current repository does not include archived outputs satisfying this
manifest. The documented historical outcomes should therefore be treated as
qualitative until corresponding artifacts are released.