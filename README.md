# System Programming Lab 11 Multiprocessing

## Overview of Implementation

Created a new file named 'mandelmovie.c' that modifies the 'mandel.c' file to create 50 mandel images, each slightly altered from the last in terms of zoom. The iteration_to_color function was modified to use the modulo to cycle RGB colors for more unique colors.

The scaling method has been changed to exponential for a smooth zoom and the default center coordinates have been changed to the Seahorse Valley values. Specify the number of children to be created using the '-c' command line argument.

Each child is created inside of a for loop and assigned an image number to create, all parents created increment processor counter to keep track of running children. Once the image is completed, the child terminates and a new child is created for the next image as needed.

## Runtime Graph

![Runtime Graph](https://github.com/MSOE-CPE2600/multiprocessing-turney-kruschkaa/blob/main/RuntimeGraph.png?raw=true)

## Results

Looking at the graph above, the time taken to complete the runtime roughly halves when moving from 1 -> 2 -> 5 -> 10 children, although the jump from 5 to 10 is noticeably less. When moving from 10 -> 20 chilren processes, there is a minimal decrease in time needed to run the program as it hits the relistic limit of how much processing time can be decreased for the program.