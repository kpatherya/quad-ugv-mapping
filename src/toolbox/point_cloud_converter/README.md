# point_cloud_converter

A small ROS 2 adapter from `sensor_msgs/msg/PointCloud` to
`sensor_msgs/msg/PointCloud2`.

The node preserves the input message's timestamp and coordinate frame. It does
not transform points. Use TF-aware processing before the converter when the
output must be expressed in a different frame.

## Run

```bash
ros2 run point_cloud_converter point_cloud_converter_node --ros-args \
  -p points_in:=/vins_estimator/point_cloud \
  -p points2_out:=/point_cloud_converter/output
```

| Parameter | Default | Meaning |
| --- | --- | --- |
| `points_in` | `/vins_estimator/point_cloud` | Input `PointCloud` topic |
| `points2_out` | `/point_cloud_converter/output` | Output `PointCloud2` topic |

The package was imported with unresolved maintainer and license metadata. See
the repository provenance guide before redistributing it independently.