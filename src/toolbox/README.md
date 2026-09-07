# Toolbox packages

Small ROS packages maintained as part of this research toolbox live here.
Packages should expose configurable topics and frames, include tests for their
core behavior, and document data contracts independently of a specific robot.

The historical `point_cloud_converter` bridges `sensor_msgs/PointCloud` to
`sensor_msgs/PointCloud2` for the mapping pipeline. Its inherited package
metadata is retained pending a provenance review; do not infer authorship from
its location in this directory.