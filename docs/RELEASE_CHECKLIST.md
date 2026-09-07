# Public Release Checklist

The GitHub repository should remain private until the ownership and licensing
items below are resolved. A public GitHub URL alone does not make mixed-license
source safely reusable.

## Authorization and provenance

- [ ] Obtain written approval from the Lunar Lab or the appropriate repository
  owner to republish the snapshots imported from the private lab repository.
- [ ] Confirm how Prof. Lu Gan and Sandilya Sai Garimella want supervision and
  mentorship credited.
- [ ] Recover exact upstream commit IDs for every directory in `src/vendor` and
  `src/integrations`.
- [ ] Identify the original source and license of `point_cloud_converter`.
- [ ] Replace all `TODO` package license fields using authoritative upstream
  license information.
- [ ] Verify the bundled DBoW, DVision, BRIEF, and camera-model notices within
  the VINS-Mono integration.

## Repository hygiene

- [ ] Scan the full Git history for credentials, private dataset paths, personal
  data, and restricted research artifacts.
- [ ] Decide whether the 57 MB `brief_k10L6.bin` vocabulary should use Git LFS,
  a release asset, or a checksum-verified download script.
- [ ] Remove unrelated vendored systems or explain their role in the supported
  workflow.
- [ ] Build the `mapping` profile in the documented container on Linux amd64.
- [ ] Run at least one EuRoC sequence and archive commands, logs, trajectories,
  parameter files, checksums, and evaluation output.
- [ ] Run a recorded RealSense bag and verify the topic and TF contracts.

## GitHub release settings

- [ ] Keep the About description and focused topics.
- [ ] Enable Issues for reproducibility reports.
- [ ] Add a social preview only if it shows an actual reconstruction or robot
  setup and has permission for public use.
- [ ] Connect Zenodo, reserve a DOI, and add the DOI to `CITATION.cff` only after
  the first immutable release is archived.
- [ ] Create a signed `v0.1.0` release from a clean, validated commit.
- [ ] Change visibility to public only after all authorization checks pass.