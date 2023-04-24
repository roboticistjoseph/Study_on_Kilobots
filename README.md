# Kilobots: Singles vs DUO's

## Independent Research Study:

Unleashing the Swarm: Exploring Kilobot Performance in Forming Formations as Singles vs Duos. Join our groundbreaking research study to unlock the secrets of these small but mighty robots. Witness the mesmerizing dance of kilobots as they shape-shift into formations. Will they outshine as lone stars or synergize as dynamic duos? Be part of this cutting-edge investigation into the fascinating world of kilobots and be amazed!

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

---
## Power Squad:
- [Joseph Pranadeer Reddy Katakam](https://github.com/roboticistjoseph)
- [Jared Allanigue](https://www.linkedin.com/in/jared-allanigue)
- [Jack Mirenzi](https://www.linkedin.com/in/jack-mirenzi)

Author:  Joseph Pranadeer Reddy Katakam (jkatak73@terpmail.umd.edu)

## Table of Contents
1. [Introduction](#intro-to-kilobots)
2. [KiloGUI Setup](#instructions-to-setup-kilogui-and-connect-to-controller)
3. [Package Description](#about-this-package)
4. [Repository Breakdown](#package-breakdown)
5. [Add new Code](#instructions-to-add-new-code)
6. [Build Package](#instructions-to-build-a-program)
7. [Error Guide](#errors-when-building-a-file-using-make)


## Intro to Kilobots:
Kilobots are low-cost swarm robots designed to execute commands in groups of up to a thousand, simulating swarms of insects, and performing tasks that cannot be done by individual robots, developed by Harvard University's Radhika Nagpal and Michael Rubenstein, capable of collective transport.

## Instructions to Setup KiloGUI and Connect to Controller:
- To set up KiloGUI (software to upload code onto kilobot), there is a file called '[Kilobot GUI Setup_Guide.pdf](/assets/Kilobot_GUI_Setup_Guide.pdf)' in the 'assets' folder located in the main directory.
- Once everything has been set up, everytime you can proceed to Step (5-7) in Section III of the PDF to connect the Arduino controller with the PC.

## About this Package:
- This package is a collection of properly arranged directories that are designed to convert '.c' files into the required '.hex' format for conducting experiments on kilobots.
- The kilobots can only execute files in the '.hex' format.

### How the Package works:
- When you use the 'make' command to build this folder, it will generate 
    three folders automatically: 'build', 'docs', and 'hex'.
- Among these folders, the 'hex' folder contains a list of '.hex' programs that can be uploaded onto the kilobot by using 'KiloGUI'.

## Package Breakdown:

    1. 'assets':
        - The "assets" folder holds additional documents that can help with understanding how to work with Kilobots.

    2. 'kilolib':
        - "kilolib" is a library developed by Harvard University that provides a useful API for programming Kilobots.
        - The 'kilolib' files in this package have been properly documented for better comprehension of API.
        - For a better comprehension of the 'kilolib' library, would recommend going throught the tutorials from 'https://kilobotics.com'.

    3. 'src':
        - The "src" folder contains sub-folders that correspond to each individual project.
        - Although it is possible to put the code directly in the 'src' folder, doing so would result in a cluttered package.
        - NOTE: The folder names in "src" should be in lower-cased snake_case. If not, the package might run into building issues.

    4. 'MAKE':
        - The "MAKE" file is responsible for building the files included in this package.
        - The file has been modified so that it can search the whole package for the requested file and 
        build it using 'kilolib'.

## Instructions to add New Code:
- To include a new code, make a directory within the 'src' folder named after the project, and then put the corresponding '.c' files inside it.

## Instructions to Build a Program:
    1. Open the terminal and navigate to the source directory that contains the MAKE file.
    2. Assuming that the code to be constructed is named 'file_name.c', make sure it can be found in 
    one of the project directories located in 'src'.
    3. Enter the following command: "make FILENAME=file_name.c"
    4. This will create a "hex" folder in the source directory, which contains the "file_name.hex" file.
    5. Use KiloGUI to upload the "file_name.hex" file to your kilobot.

## Errors when building a file using 'make':
- Sometimes, when trying to generate multiple '.hex' files, there could be a built system files overlap.
- This would throw out error such as, "Nothing to be done for 'all'".
- In such a case, delete the build folder generated in the source directory and try rerunning the 'make' command.