FROM osrf/ros:foxy-desktop

SHELL ["/bin/bash", "-c"]

RUN apt-get update && apt-get install -y --no-install-recommends \
      python3-colcon-common-extensions \
      python3-rosdep \
      python3-vcstool \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . /workspace

RUN source /opt/ros/foxy/setup.bash \
    && rosdep update \
    && rosdep install --from-paths src --ignore-src --rosdistro foxy -r -y \
    && ./tools/build_workspace.sh mapping

ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]