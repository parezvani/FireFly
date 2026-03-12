# ESP-32-Drone (FireFLY)

FireFLY is a student-built drone platform centered on the `ESP32-C3-DevKit-RUST-1` for fire risk monitoring in areas without existing sensor coverage.

## Need
Remote and high fire-risk areas lack the sensor infrastructure necessary to monitor environmental conditions during the critical pre-fire window, leaving fire response teams without the real-time data needed to act early.

## Goal
Provide fire response teams with real-time temperature, humidity, and smoke readings from high-risk, infrastructure-free terrain by deploying a rapidly mobilized monitoring system that transmits real-time data to a ground station and alerts operators when fire risk thresholds are crossed.

## Repository Layout
- `main/`: source code, firmware experiments, and hardware tests.
  - `scripts/`: ESP-NOW communication and control prototypes.
  - `base-server/fire-server/`: Rust ground-station server/client for telemetry.
  - `testing/motor/`: motor test firmware and hardware validation code.
  - `lab4_1/`: earlier lab firmware kept for reference.
  - `scrapped/`: older discarded experiments and scratch files.
- `docs/`: project documentation and design artifacts.
  - `drone-designs/`: CAD models, frame iterations, and dimensions.
  - `circuit-design/`: wiring diagrams, schematics, and circuit photos.
  - `bill-of-materials/`: BoM revisions and pricing snapshots.
  - `design-document/`: LaTeX source and exported design document PDFs.
  - `status-reports/`: progress reports and planning artifacts.
  - `meeting-notes/`: team notes, decisions, and action items.
  - `research-notes/`: reference material gathered during development.
- `esp/esp-idf/`: local checkout of the ESP-IDF framework used by the embedded work.
