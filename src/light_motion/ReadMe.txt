Project:
    - Kilobot approaches a source of light which may be dynamically moving at a very slow rate.

Author:
    Joseph Pranadeer Reddy Katakam (jkatak73@terpmail.umd.edu)

Resource:
    - https://kilobotics.com/labs#lab5-move-to-light


Intro:
    - Using the ambient light sensor to measure the environmental conditions and follow a gradient of light.
    - This code is very sensitive to light levels.
    - As such, the code is written to eliminate noise by sampling the readings.
    Threshold levels might have to be adjusted to work in an individual's environment.

Flowchart:
    - Check it out in the assets folder.

How to run:
    - Run the below command from source directory.
        - 'make FILENAME=move_to_light.c'
    - The '.hex' version of the file can be found in 'hex' folder in source directory.
    - Upload it to the kilobot using the 'KiloGUI'.

Output:
    - Kilobot follows the light source.

Note:
    - Perform the experiment in a dark room with a single light source.