# Notes

## Meeting Goals
- Testing goals

## Testing
- Design document / testing
  - "Sensors should work at range ... and outside at range ..."
  - 5 items
    - Need/Goal
    - Design objectives (02.01)
      - Measurable
      - Transmission range
      - Transmission rate
      - Battery life
      - Temperature range
    - Personas (02.02)
    - Testing plans
    - The design
- Numerical testing
- App testing
  - Able to connect app and ESP through computer's connection
  - Cut from design

## Changes
- Review design document
  - Need/Goal statement memorized
  - Remove live transmission
- Design
  - Remove phone/app
- Phone/App Stage -> Computer
  - Cut stage and directly from connect to a computer and server
  - Program commands like "Fly vertically 1 meter and hover"
  - Call functions to grab/curl data
- Testing
  - Communicate with ESP1 to ESP2 and have ESP2 have a flashing/blinking light
  - ESP-NOW
- Script
  - 15 mins + 5 mins of Q&A
  - Explain the 5 items
  - Don't need to answer all the questions

## Summary / Closing
- Server <- Computer -> ESP1 -> (ESP-NOW) -> ESP2 -> Blinking LED on ESP2
- Changed design from phone to computer via ESP-NOW
- Focus on Need/Goal
- No mention of ESP

## Design Objectives
- "Quantifiable expectations of performance" that you aim for, or try to achieve
- Identify performance characteristics that are of most interest to the client
- Describe those characteristics in a way "you and the client" can "decide" if the design meets expectations
- Describe the conditions under which a design will operate
- Example
  - Objective:
  - Design an inexpensive to manufacture battery carrier that does not reduce battery life, can be quickly replaced by Apple Store "Geniuses", and is cost-effectively recyclable
