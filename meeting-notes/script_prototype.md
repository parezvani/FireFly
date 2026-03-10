# Prototype Demonstration Script (Dry Run)

## Total Time
- Target length: 20 minutes
- Slide shown for entire demo: `circuit-design/Schematic_Design.png`

## Speaker 1 - Opening (Need + Goal)
"Hi everyone, we are Team FireFly. Our project addresses one core need: remote and high fire-risk areas often have no fixed sensor infrastructure, so responders do not get enough pre-fire data early enough to act safely and quickly.

Our goal is to provide real-time temperature, humidity, and smoke readings from those infrastructure-free areas using a rapidly deployable drone platform. During this prototype demonstration, we are showing that the core architecture is feasible: power and propulsion control, onboard processing, and wireless data flow to operators."

## Speaker 2 - Schematic Walkthrough (Electrical/Control Path)
"I will walk through the schematic from left to right.

On the left side, the battery provides system power. That power feeds the ESC, and the ESC controls motor output. In the final configuration, the microcontroller sends control signals to the ESC, and the ESC drives the brushless motors and propellers.

In our current prototype test, we demonstrate this control principle using an ESP32 with an H-bridge and DC motor test rig while we wait for the replacement ESC. We can show speed and direction control, which validates our control path logic before full brushless integration.

At the center is the MCU, which is the main decision point. It receives data from sensors and sends control and telemetry data to the rest of the system."

## Speaker 3 - Mechanical/Design Integration
"From the mechanical side, our frame is designed so the electronics path shown in this schematic can be assembled cleanly and serviced quickly. The battery, controller, and wiring routes are planned to minimize cable clutter and reduce rework during testing.

Motor pod and frame iterations were used to improve fit, internal space, and wire routing access. This lets us safely iterate electrical and software tests now, then move to integrated flight testing once the final motor-control hardware is installed."

## Speaker 4 - Software/Data Path + Live Prototype Evidence
"On the right side of the schematic is the operator and data pipeline.

The MCU exchanges control and telemetry with the phone/app side, and data can be forwarded to a server for storage and monitoring. We have already validated core communication pieces through packet tests and logging workflows, including UDP traffic checks and message visibility during testing.

For sensing, the final architecture uses temperature/humidity plus smoke sensing for wildfire risk monitoring. In the prototype phase, we are validating sensor-read and communication behavior in modular steps so each subsystem is testable before full integration.

The top path in the schematic shows future expansion through drone-to-drone or mesh-style communication, so this architecture can scale beyond a single unit when needed."

## Speaker 1 - Close (What This Demo Proves)
"To close, this dry run demonstrates three things.

First, the architecture is coherent from battery and control logic through actuation and data delivery.
Second, we have working prototype evidence for motor control behavior and communication behavior, even while final ESC-dependent integration is pending.
Third, this prototype supports our project objective: infrastructure-independent pre-fire data collection to improve responder awareness and safety.

Our next step is completing full hardware integration with the incoming parts and running end-to-end tests with the finalized sensor and propulsion stack."

## Backup One-Liners for Q&A
- "Why use a DC motor in the demo?" -> "It is a controlled stand-in to validate signal-to-actuation behavior while waiting for the replacement ESC for brushless integration."
- "What is working today?" -> "Control signaling, motor behavior in the test rig, and communication packet testing are working today."
- "What is the main risk right now?" -> "Hardware arrival timing for final ESC integration"
