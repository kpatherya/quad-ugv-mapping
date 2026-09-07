# Contributing

Contributions that improve portability, calibration transparency, tests, or
dataset-backed evaluation are welcome.

## Before opening a change

1. Decide whether the change belongs in `src/toolbox`, `src/integrations`, or an
   upstream project currently mirrored in `src/vendor`.
2. Open generic vendor fixes upstream first when possible.
3. Do not remove author, copyright, or license notices.
4. Do not commit bags, datasets, generated maps, credentials, or machine-local
   paths.

## Validation

Run the platform-independent checks on every change:

```bash
python3 tools/validate_repository.py
bash -n tools/build_workspace.sh
```

On Ubuntu 20.04 with ROS 2 Foxy, also run the narrowest relevant profile:

```bash
source /opt/ros/foxy/setup.bash
./tools/build_workspace.sh converter
./tools/build_workspace.sh vio
./tools/build_workspace.sh mapping
```

For runtime changes, include the dataset or bag checksum, exact launch command,
parameter files, topic graph, TF tree, and before/after result. Hardware changes
must identify the sensor model, firmware, calibration, and stream settings.

## Pull requests

Keep third-party updates separate from project integration changes. Explain the
behavioral contract being changed and provide a command that would fail before
the change and pass afterward. Claims about accuracy or robustness require a
committed evaluation method and inspectable result artifacts.