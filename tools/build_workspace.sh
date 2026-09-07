#!/usr/bin/env bash
set -euo pipefail

profile="${1:-mapping}"

if ! command -v colcon >/dev/null 2>&1; then
  echo "colcon is required. Source a ROS 2 Foxy environment first." >&2
  exit 127
fi

case "${profile}" in
  converter)
    packages=(point_cloud_converter)
    ;;
  vio)
    packages=(camera_model config_pkg feature_tracker vins_estimator pose_graph benchmark_publisher ar_demo)
    ;;
  mapping)
    packages=(feature_tracker vins_estimator pose_graph point_cloud_converter octomap_server)
    ;;
  all)
    packages=()
    ;;
  *)
    echo "Usage: $0 {converter|vio|mapping|all}" >&2
    exit 2
    ;;
esac

if [[ -n "${ROS_DISTRO:-}" && "${ROS_DISTRO}" != "foxy" ]]; then
  echo "Warning: this snapshot targets ROS 2 Foxy; active distro is ${ROS_DISTRO}." >&2
fi

build_args=(--symlink-install --event-handlers console_direct+)
if ((${#packages[@]})); then
  build_args+=(--packages-up-to "${packages[@]}")
fi

colcon build "${build_args[@]}"