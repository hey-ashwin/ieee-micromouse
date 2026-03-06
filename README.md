## IEEE Micromouse
Autonomous maze-solving robot built from scratch for the IEEE Micromouse competition at BITS Pilani APOGEE ’23 & ’24.
The robot explores and maps a 16×16 maze using the Flood Fill algorithm to compute the shortest path to the goal.

Micromouse is a robotics challenge in which an autonomous robot must navigate an unknown maze and reach the center in the shortest possible time.
This project involved designing both the hardware platform and the embedded control software required for autonomous maze exploration and navigation.

### Operation

Micromouse competitions typically involve two stages:

1. Exploration Run
During the first run, the robot explores the maze slowly while mapping walls and paths using IR sensor arrays. The maze structure is stored and processed using the Flood Fill algorithm to determine optimal routes.

2. Speed Run
After exploration, the final timed run begins. The robot autonomously computes the optimal shortest path and traverses it at higher speeds to reach the goal as quickly as possible.

Ongoing Improvements - 
PID control for motor speed regulation and heading stabilization is currently being implemented to improve navigation accuracy and motion stability.
