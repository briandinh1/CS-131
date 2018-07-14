#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <queue>
 
/* Global variables, Look at their usage in main() */
int image_height;
int image_width;
int image_maxShades;
int inputImage[1000][1000];
int outputImage[1000][1000];
int num_threads; 
int chunkSize;
int maxChunk;

std::queue<int> chunks;
const int maskX[3][3] = { { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } };
const int maskY[3][3] = { { 1, 2, 1 }, { 0, 0, 0 }, { -1, -2, -1 } };


/* ****************Change and add functions below ***************** */
void sobel_filter(int pos)
{
	int limit = (pos + maxChunk < image_height) 
		? pos + maxChunk 
		: image_height;
	
	for (int x = pos; x < limit; ++x)
	{
		for (int y = 0; y < image_width; ++y)
		{
			int sumx = 0;
			int sumy = 0;
			int sum = 0;
			
			/* For handling image boundaries */
			if (x == 0 || x == (image_height - 1) || y == 0 || y == (image_width - 1))
			{				
				outputImage[x][y] = 0;
				continue;
			}
			else
			{
				/* Gradient calculation in X Dimension */
				for (int i = -1; i <= 1; ++i) 
					for (int j = -1; j <= 1; ++j)
						sumx += (inputImage[x + i][y + j] * maskX[i + 1][j + 1]);
				
				/* Gradient calculation in Y Dimension */
				for (int i = -1; i <= 1; ++i) 
					for (int j = -1; j <= 1; ++j)
						sumy += (inputImage[x + i][y + j] * maskY[i + 1][j + 1]);

				/* Gradient magnitude */
				sum = (abs(sumx) + abs(sumy));
			}
			
			outputImage[x][y] = (sum < 0) ? 0 : ((sum > 255) ? 255 : sum);
		}
	}
}


int get_dynamic_chunk()
{
	int nextAvailableChunk = chunks.front();
	chunks.pop();
	return nextAvailableChunk;
}


void compute_chunk()
{
	while(!chunks.empty()) 
		sobel_filter(get_dynamic_chunk());
}


void dispatch_threads()
{
	std::vector<std::thread> threads;
	
	for (int i = 0; i < image_height; i += maxChunk) 
		chunks.push(i);
	
	for (int i = 0; i < num_threads; ++i)
		threads.push_back(std::thread(compute_chunk));

	for (int i = 0; i < num_threads; ++i)
		threads[i].join();
}



/* ****************Need not to change the function below ***************** */

int main(int argc, char* argv[])
{
    if(argc != 5)
    {
        std::cout << "ERROR: Incorrect number of arguments. Format is: <Input image filename> <Output image filename> <Threads#> <Chunk size>" << std::endl;
        return 0;
    }
 
    std::ifstream file(argv[1]);
    if(!file.is_open())
    {
        std::cout << "ERROR: Could not open file " << argv[1] << std::endl;
        return 0;
    }
    num_threads = std::atoi(argv[3]);
    chunkSize  = std::atoi(argv[4]);

    std::cout << "Detect edges in " << argv[1] << " using " << num_threads << " threads\n" << std::endl;

    /* ******Reading image into 2-D array below******** */

    std::string workString;
    /* Remove comments '#' and check image format */ 
    while(std::getline(file,workString))
    {
        if( workString.at(0) != '#' ){
            if( workString.at(1) != '2' ){
                std::cout << "Input image is not a valid PGM image" << std::endl;
                return 0;
            } else {
                break;
            }       
        } else {
            continue;
        }
    }
    /* Check image size */ 
    while(std::getline(file,workString))
    {
        if( workString.at(0) != '#' ){
            std::stringstream stream(workString);
            int n;
            stream >> n;
            image_width = n;
            stream >> n;
            image_height = n;
            break;
        } else {
            continue;
        }
    }

    /* maxChunk is total number of chunks to process */
    maxChunk = ceil((float)image_height/chunkSize);

    /* Check image max shades */ 
    while(std::getline(file,workString))
    {
        if( workString.at(0) != '#' ){
            std::stringstream stream(workString);
            stream >> image_maxShades;
            break;
        } else {
            continue;
        }
    }
    /* Fill input image matrix */ 
    int pixel_val;
    for( int i = 0; i < image_height; i++ )
    {
        if( std::getline(file,workString) && workString.at(0) != '#' ){
            std::stringstream stream(workString);
            for( int j = 0; j < image_width; j++ ){
                if( !stream )
                    break;
                stream >> pixel_val;
                inputImage[i][j] = pixel_val;
            }
        } else {
            continue;
        }
    }

    /************ Function that creates threads and manage dynamic allocation of chunks *********/
    dispatch_threads();

    /* ********Start writing output to your file************ */
    std::ofstream ofile(argv[2]);
    if( ofile.is_open() )
    {
        ofile << "P2" << "\n" << image_width << " " << image_height << "\n" << image_maxShades << "\n";
        for( int i = 0; i < image_height; i++ )
        {
            for( int j = 0; j < image_width; j++ ){
                ofile << outputImage[i][j] << " ";
            }
            ofile << "\n";
        }
    } else {
        std::cout << "ERROR: Could not open output file " << argv[2] << std::endl;
        return 0;
    }
    return 0;
}