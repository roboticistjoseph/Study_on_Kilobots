Project:
    - Perform basic communication between two kilobots.

Author:
    Joseph Pranadeer Reddy Katakam (jkatak73@terpmail.umd.edu)

Resource:
    - https://kilobotics.com/labs#lab2-communication


Intro:
    - For the communication here, two kilobots are used, one as transmitter and one as receiver.
    - using 'transmitter_bot.c' one kilobot acts as a speaker.
    - using 'receiver_bot.c' another kilobot acts as a listener.
    - According to kilobotics website, every kilobot attempts to send message twice per sec.
    This can be verified from the Output.

Flowchart:
    - Check it out in the assets folder.

How to run:
    - Two hex files have to be generated in order to upload to two different kilobots.
    - Run both the below commands from source directory.
        1. Speaker: 'make FILENAME=transmitter_bot.c'
        2. Listener: 'make FILENAME=receiver_bot.c'
    - The '.hex' version of these files can be found in 'hex' folder in source directory.
    - Upload them to the kilobots using the 'KiloGUI'.

Output:
    - Speaker blinks 'Megenta' everytime it sends a messgae.
    - Listener blinks 'Yellow' everytime it receives a message.

Note:
    - When performing the experiment, both the kilobots should be in communicating range