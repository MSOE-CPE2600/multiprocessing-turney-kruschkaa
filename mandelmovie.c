/*
 * File: mandelmovie.c
 * Brief: Creates 50 mandel images with a user defined number of children for processing,
 * then can be compled into a movie by running "ffmpeg -i mandel%d.jpg mandel.mpg"
 * 
 * Author: Alexander Kruschka
 * Assignment: Lab 11
 * Section: 111
 * Date: 11/18/25
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include "jpegrw.h"
#define NUM_IMAGES 50

static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
static void compute_image( imgRawImage *img, double xmin, double xmax,
									double ymin, double ymax, int max, int num_threads);
static void show_help();

typedef struct {
	int start_y;
	int end_y;
	int width;
	int height;
	imgRawImage* img;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int max;
} thread_data;

int main(int argc, char *argv[])
{
    // Define default variables to be used in program
    char input;
	double xcenter = -0.74364388703;
    double ycenter = 0.13182590420;
	double xscale = 0.2;
	double yscale = 0;
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;
    int    max_children = 10;
    int    num_procs = 0;
	int    num_threads = 1;

    // Parse input
    while((input = getopt(argc,argv,"x:y:s:W:H:m:c:t:h"))!=-1) {
		switch(input) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
            case 'c':
                max_children = atoi(optarg);
                break;
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

    // Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
	yscale = xscale / image_width * image_height;

    // Run children processes, each creating one image, in a loop until 50 images done
    for (int i = 0; i < NUM_IMAGES; i++)
    {
        if (num_procs >= max_children)
        {
            wait(NULL);
            num_procs--;
        }

        // Split into child and parent
        int pid = fork();

        // PID error checking
        if (pid == -1)
        {
            printf("Error when forking to create image %d", i);
            exit(EXIT_FAILURE);
        }

        // Check if child and, if so, create mandel image with offset based on image number
        if (pid == 0)
        {
            // Declare outfile for this instance
            char outfile[20];
            snprintf(outfile, sizeof(outfile), "mandel%d.jpg", i);

            // Modify the scales exponentially based on image/fram number for zooming in smoothly
			double zoom_factor = 0.87;
            xscale = xscale * pow(zoom_factor, i);
            yscale = yscale * pow(zoom_factor, i);

            // Display the configuration of the image.
			printf("mandel %d: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s\n",i,xcenter,ycenter,xscale,yscale,max,outfile);
            
            // Create a raw image of the appropriate size.
            imgRawImage* img = initRawImage(image_width,image_height);

            // Fill it with a black
            setImageCOLOR(img,0);

            // Compute the Mandelbrot image
            compute_image(img,xcenter-xscale/2,xcenter+xscale/2,ycenter-yscale/2,ycenter+yscale/2,max, num_threads);

            // Save the image in the stated file.
            storeJpegImageFile(img,outfile);

            // Exit child process
            exit(0);
        }

        // Check if parent and, if so, increment processor counter.
        if (pid > 0)
        {
            num_procs++;
        }
    }

    while (num_procs > 0)
    {
        wait(NULL);
        num_procs--;
    }

    return 0;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Calculate the pixels for a given part of a Mandelbrot image with a start and end y value. 
Intended for use as the function of a thread that merges to a whole image. Limiting interations
to "max"
*/
void *calc_imag_part(void *arg)
{
	// Get and assigne variables to be used for this specific thread process
	thread_data *data = (thread_data*)arg;
	imgRawImage* img = data->img;
	int j, i;
	int width = data->width;
	int height = data->height;
	int max = data->max;
	double xmin = data->xmin;
	double xmax = data->xmax;
	double ymin = data->ymin;
	double ymax = data->ymax;

	// For every pixel in the image assigned to this thread in y-range...
	for(j=data->start_y;j<data->end_y;j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			setPixelCOLOR(img,i,j,iteration_to_color(iters,max));
		}
	}

	return NULL;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/
void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int num_threads)
{
	// Initialize basic variables for data of image and splitting of values
	int width = img->width;
	int height = img->height;
	
	// Initialize struct with constant data, leaving start and end values 0 to be modified later
	thread_data data[num_threads];

	// Initialize thread with the number of threads to be used
	pthread_t thread[num_threads];
	printf("Creating %d threads to render slices of %d px.\n", num_threads, 1000/num_threads);

	// Run threads to create image with respective values
	for (int id = 0; id < num_threads; id++) {
		data[id].start_y = height * ((float)id / num_threads);
		data[id].end_y = height * ((float)(1 + id) / num_threads);
		data[id].width = width;
		data[id].height = height;
		data[id].img = img;
		data[id].xmin = xmin;
		data[id].xmax = xmax;
		data[id].ymin = ymin;
		data[id].ymax = ymax;
		data[id].max = max;

		pthread_create(&thread[id], NULL, calc_imag_part, &data[id]);
	}

	// Wait for all threads before proceeding
	for (int id = 0; id < num_threads; id++) {
		pthread_join(thread[id], NULL);
	}
}


/*
Convert a iteration number to a color.
*/
int iteration_to_color( int iters, int max )
{
	// If we hit the max, we are inside the set, so make it BLACK.
    if (iters >= max) return 0x000000;

    // Use modulo to cycle colors. Multipliers (13, 21, 34) are arbitrary primes.
    int r = (iters * 13) % 255;
    int g = (iters * 21) % 255;
    int b = (iters * 34) % 255;

    // Combine into hex integer (0xRRGGBB)
    return (r << 16) | (g << 8) | b;
}

// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-c <value>  Set number of child processes. (default=10)\n");
	printf("-t <value>  Set number of pthreads per image. (default=1)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}