# Provenance and Attribution

## Why this document exists

The original lab repository committed complete dependency trees in two snapshot
imports on 2025-08-25. Their former nested Git metadata and exact upstream commit
IDs were not retained. The checked-in files are reproducible through this
repository's commit history, but they are not a substitute for upstream release
provenance.

Directory placement describes maintenance responsibility, not authorship.

## Research contribution represented here

Kausar Patherya's documented contribution was ROS 2 systems integration,
dependency and TF debugging, EuRoC validation, and RealSense/room-mapping
experimentation under the supervision of Prof. Lu Gan and graduate mentor
Sandilya Sai Garimella. The Git history also records the original source import
by Sandilya and the initial top-level README by Kausar.

This repository is not evidence that Kausar authored the upstream VINS-Mono,
OpenVINS, OctoMap, RealSense, PX4, or ROS vision algorithms.

## Component inventory

| Local path | Upstream | Role | Declared license | Snapshot precision |
| --- | --- | --- | --- | --- |
| `src/integrations/vins_mono_ros2` | [dongbo19/VINS-MONO-ROS2](https://github.com/dongbo19/VINS-MONO-ROS2) | Adapted ROS 2 VINS pipeline and project wiring | GPLv3 in upstream README; package manifests normalized to `GPL-3.0-only` | Exact upstream commit unknown |
| `src/vendor/octomap_mapping` | [OctoMap/octomap_mapping](https://github.com/OctoMap/octomap_mapping) | ROS wrappers and OctoMap server | BSD | Exact upstream commit unknown |
| `src/vendor/octomap_msgs` | [OctoMap/octomap_msgs](https://github.com/OctoMap/octomap_msgs) | OctoMap ROS interfaces | BSD | Exact upstream commit unknown |
| `src/vendor/open_vins` | [rpng/open_vins](https://github.com/rpng/open_vins) | Alternative VIO implementation and reference configurations | GPLv3 | Exact upstream commit unknown |
| `src/vendor/realsense-ros` | [IntelRealSense/realsense-ros](https://github.com/IntelRealSense/realsense-ros) | RealSense ROS 2 driver | Apache-2.0 | Exact upstream commit unknown |
| `src/vendor/px4-ros2-interface-lib` | [Auterion/px4-ros2-interface-lib](https://github.com/Auterion/px4-ros2-interface-lib) | PX4 ROS 2 interface | See component `LICENSE` | Exact upstream commit unknown |
| `src/vendor/px4_msgs` | [PX4/px4_msgs](https://github.com/PX4/px4_msgs) | PX4 message definitions | BSD-3-Clause | Exact upstream commit unknown |
| `src/vendor/vision_opencv` | [ros-perception/vision_opencv](https://github.com/ros-perception/vision_opencv) | `cv_bridge` and camera geometry | Apache-2.0 and BSD | Exact upstream commit unknown |
| `src/toolbox/point_cloud_converter` | Historical ROS package source not yet identified | PointCloud compatibility adapter | `LicenseRef-Provenance-Pending` in package manifest | Provenance review required |

VINS-MONO-ROS2 also includes VINS-Mono, camera-model, DBoW, DVision, and BRIEF
derived code. Consult its README and source headers before redistribution.

## Licensing policy

- Component license files and source headers override the root license.
- The root BSD-3-Clause terms apply only where no more specific license exists
  and only to original toolbox material.
- GPL-covered executables and derivative source must remain compliant with GPL
  source-distribution requirements.
- Do not replace upstream maintainer or author fields merely to signal current
  repository maintenance.

This inventory is engineering documentation, not legal advice.

## Maintainer follow-up

Before a formal archival release:

1. Identify the exact upstream commit for every imported snapshot using file
   hashes or the original clone records.
2. Confirm every VINS-MONO-ROS2 package manifest license declaration against
  authoritative upstream notices without removing third-party attribution.
3. Identify the source and license of `point_cloud_converter`.
4. Add a machine-readable dependency manifest pinned to verified commits.
5. Archive datasets and result artifacts separately with checksums.