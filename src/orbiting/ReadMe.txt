Project:
    - Make one kilobot orbit around another stationary kilobot while keeping a fixed distance.

Author:
    Joseph Pranadeer Reddy Katakam (jkatak73@terpmail.umd.edu)

Resource:
    - https://kilobotics.com/labs#lab4-orbit


Intro:
    - For the communication here, two kilobots are used, one as transmitter and one as receiver.
    - using 'orbit_star.c' one kilobot acts as a star (speaker).
    - using 'orbit_planet.c' another kilobot acts as a planet (listener).
    - Here the edge following algorithm is implemented using distance sensing.
    - Kilobots communication range is (33mm - 110mm). Therefore, an ideal fixed distance of 60mm is chosen.

Flowchart:
    - Check it out in the assets folder.

How to run:
    - Two hex files have to be generated in order to upload to two different kilobots.
    - Run both the below commands from source directory.
        1. Star: 'make FILENAME=orbit_star.c'
        2. Planet: 'make FILENAME=orbit_planet.c'
    - The '.hex' version of these files can be found in 'hex' folder in source directory.
    - Upload them to the kilobots using the 'KiloGUI'.

Output:
    - Planet kilobot orbits around Star kilobot.

Note:
    - When performing the experiment, both the kilobots should be in communicating range.