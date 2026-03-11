# Prototype Demonstration Script (Mar. 10 Revision)

## Total Time
- Target length: 15 minutes
- Q&A: 5 minutes
- Main visual for most of the demo: `circuit-design/Schematic_Design_2.png`
- Required flow:
  - Need/Goal
  - Design Objectives
  - Personas
  - Testing Plan
  - The Design

## Opening (Need + Goal)
"Hi everyone, we are Team FireFly.

The need we are addressing is that remote and high fire-risk areas often do not have fixed sensor infrastructure during the critical pre-fire window. Because of that, fire response teams can be left without timely environmental data when they need it most.

Our goal is to provide a rapidly deployable monitoring system that can gather temperature, humidity, and smoke information in infrastructure-free terrain and return that information to operators so they can make earlier and safer decisions.

For this prototype demonstration, we are showing that the core communication and control architecture can be tested in modular pieces and still map directly back to that need and goal." 

## Design Objectives
"Our design objectives are measurable, because we need a way to decide whether the design actually meets expectations.

First, the system needs enough transmission range to operate away from the operator in realistic outdoor conditions.
Second, the transmission rate needs to be fast enough that commands and environmental readings stay useful.
Third, battery life needs to support a meaningful monitoring mission, not just a very short bench test.
Fourth, the system needs to operate across the temperature conditions expected in wildfire monitoring environments.

These objectives matter because they turn the project from an idea into something we can test numerically and improve with evidence." 

## Personas
"This design is meant for users across a wide experience range.

One example user is a wildfire operations coordinator who needs a system that is fast to deploy, easy to understand, and reliable under time pressure.
Another example is a researcher who wants environmental data collected in remote areas and forwarded into a computer-based workflow for logging and analysis.

Mentioning personas is important because it explains why our design emphasizes clear operation, practical deployment, and a simple data path instead of extra features that do not help the user in the field." 

## Testing Plan + Prototype Evidence
"Our testing plan is to validate one functional part of the high-level design at a time.

For today's prototype, we focus on the communication path. A computer sends a counter over USB to the ground-station controller. That message is transmitted over the wireless link to the controller. The controller responds by updating a counter received .

That Counter is the visible proof in this demo. It shows that a message traveled across the intended link and triggered a response on the receiving side.

## The Design
"Using the schematic, we can connect that prototype back to the full design.

On the left side is the power and actuation path: the battery, the electronic speed controller, the motors, and the propellers.
At the center is the drone controller, which acts as the decision point for sensing, communication, and control.
On the right side is the operator path: a computer connects by USB to a ground-station controller, and the computer can also connect to a server for logging and monitoring.

In the final system, environmental readings would move from the sensors into the drone controller, across the wireless link, and back to the ground side for display and storage. In today's prototype, we use a command packet from a microcontroller and another microcontroller to control a motor." 

## Close
"To close, this prototype demonstrates five things.

First, the project is grounded in a clear need and goal.
Second, the design is being judged against measurable objectives.
Third, the system is being shaped around real users and real field constraints.
Fourth, our testing plan is evidence-based.
Fifth, the prototype result supports the overall design by proving that the communication path can produce a successful response.

Our next step is to extend that validated path into the rest of the system, including integrated sensing, motor control, and broader end-to-end testing." 

## Presenter Notes
- Keep the Need and Goal statement strong and memorized.
- Do not frame the demo around a phone or app.
- Do not name the board model directly; say `ground-station controller`, `drone controller`, `MCU`, or `wireless link`.
- Emphasize that this is a prototype test of one functional subsystem, not the full final deployment.

## Backup One-Liners for Q&A
- "Why remove the phone/app stage?" -> "The revised schematic focuses on the computer, ground-station controller, and drone controller path so we can test the core wireless link more clearly and reduce demo risk. As well as setup One to many drones to send out communications commands to"
- "What does the blinking LED prove?" -> "It proves the remote node received the message and executed the expected action."
- "What is working today?" -> "The end-to-end path from computer to ground station, across the wireless link, to the drone-side hardware response is working in the current prototype test."
- "What are you proving today?" -> "We are proving the communication architecture and the testing approach, not full flight operation."
