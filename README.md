## Wave function collapse density finder

### How to build and clean: 
 * build: make
 * clean: make clean

## Program description and how to run each program:

### Density Finder: 
This program was created to use the wave function collapse class to generate mazes. I have since taken the code and implemented it in unity to create basic level layouts for different kinds of games.
This program creates 100 different maze versions with the building blocks provided and can be set to find the most or least dense maze. 

This program was build with the capacity to run either with zero arguments and interact with a terminal based gui to make you're mazes or can be run with arguments to speed up the process

 * without arguments (for "gui" choices): ./DensityFinder

 * With arguments: ./DensityFinder <mode> <img_size> <num_threads>


#### Important note: If your img_size % num_threads isn't 0 it causes errors. To avoid user error if img_size % num_threads isn't 0, I set num_threads to 1. 


### wfcSingle: 
 * This program will produce a single maze based on the img_size provided.

 * This program requires arguments: ./wfcSingle <img_size> <num_threads>


### Argument descriptions:
 * mode - either a 1 to find a minimum density, or 2 for maximum density. 
 * img_size - the imgs are square so this is the length and width size.
 * num_threads - how many threads you wish to use.


### Program outputs:
 * Both programs provide statistical analysis to the terminal in the form of time taken for each section to perform, density of the maze outputted, and the name of the pgm that hold your results. 

 * Unless in the "gui" form of Density finder your final image will be output as "result.pgm", otherwise it will be named whatever you name you provide to the gui. 