Project:
    - Create a logical synchronous clock between different robots to 
    allow two or more robots to blink an LED in unison roughly every 4 seconds.

Author:
    Joseph Pranadeer Reddy Katakam (jkatak73@terpmail.umd.edu)

Resource:
    - https://kilobotics.com/labs#lab7-sync


Intro:
    - Method relies on averaging.
    - In order to synchronize, it collects that information and uses the average
    to make an adjustment to its own next flashing time.
    - This code is might not yield perfect output always.

Flowchart:
    - Check it out in the assets folder.

How to run:
    - Run the below command from source directory.
        - 'make FILENAME=sync.c'
    - The '.hex' version of the file can be found in 'hex' folder in source directory.
    - Upload it to the kilobot using the 'KiloGUI'.

Output:
    - All kilobots blink in sync.

Note:
    - Number of kilobots used to perform experiment is user's choice.