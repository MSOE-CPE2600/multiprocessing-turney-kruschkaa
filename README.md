# System Programming Lab 11 Multiprocessing

## Overview of Implementation

Created a new file named 'mandelmovie.c' that modifies the 'mandel.c' file to create 50 mandel images, each slightly altered from the last in terms of zoom. The iteration_to_color function was modified to use the modulo to cycle RGB colors for more unique colors. The scaling method has been changed to exponential for a smooth zoom and the default center coordinates have been changed to the Seahorse Valley values. Processes and threads have been added for better speed and efficiency.

## Process Implementation

Each child is created inside of a for loop and assigned an image number to create, all parents created increment processor counter to keep track of running children. Once the image is completed, the child terminates and a new child is created for the next image as needed. Specify the number of children to be created using the '-c' command line argument.

## Thread Implementation

Threads were done by creating a function that renders a vertical slice of an assigned image of the movie that is combined when all have completed. All of the information necessary to render the image is contained in a unique struct created and passed into the thread function for each thread. The vertical slice is calculated as an even split of the image distributed to all of the threads.

This is done in each child process, allowing each child to create a set number of thread specified by the user with the '-t' command line argument.

## Runtime Graph with Processes

![Runtime Graph](https://github.com/MSOE-CPE2600/multiprocessing-turney-kruschkaa/blob/main/RuntimeGraph.png?raw=true)

## Runtime Table with Threads and Processes

|               | 1 Thread      | 2 Threads     | 5 Threads     | 10 Threads    | 20 Threads    |
| ------------- | ------------- | ------------- | ------------- | ------------- | ------------- |
| 1 Process     | 122.337s      | 76.132s       | 43.178s       | 26.169s       | 21.671s       |
| 2 Processes   | 62.986s       | 44.633s       | 25.074s       | 22.420s       | 18.635s       |
| 5 Processes   | 31.043s       | 23.175s       | 19.306s       | 19.198s       | 19.155s       |
| 10 Processes  | 20.041s       | 18.825s       | 17.907s       | 19.083s       | 18.287s       |
| 20 Processes  | 17.921s       | 17.949s       | 17.924s       | 17.839s       | 17.734s       |


## Results

Looking at the graph, the time taken to complete the runtime roughly halves when moving from 1 -> 2 -> 5 -> 10 children, although the jump from 5 to 10 is noticeably less. When moving from 10 -> 20 chilren processes, there is a minimal decrease in time needed to run the program as it hits the relistic limit of how much processing time can be decreased for the program.

When looking at the table with threads and processes, it is clear that the number of processes had a larger impact on the total runtime than the number of threads. Essentially, multiprocessing is more effective than multiprocessing. This is likely because multiprocessing uses multiple CPU cores, allowing greater utilization of the CPU's total resources, while multithreading simply splits up the task into parallel tasks on the same core, allocating less resources for processing.

According to the table, the "sweet spot" occurs when there are 20 processes and 20 threads, but the difference in total processing time compared to all other times in the 20 process category are so similar that it could simply be run-to-run variation error. Realistically, it is likely more practical to simply assign 20 processes and a single thread per for the sake of simplicity.