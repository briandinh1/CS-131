#include "mpi.h"
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
const static int ARRAY_SIZE = 130000;
using Lines = char[ARRAY_SIZE][16];


// To remove punctuations
struct letter_only: std::ctype<char> 
{
    letter_only(): std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table()
    {
        static std::vector<std::ctype_base::mask> 
            rc(std::ctype<char>::table_size,std::ctype_base::space);

        std::fill(&rc['A'], &rc['z'+1], std::ctype_base::alpha);
        return &rc[0];
    }
};

void DoOutput(std::string word, int result)
{
    std::cout << "Word Frequency: " << word << " -> " << result << std::endl;
}

// ***************** Add your functions here *********************

int countWords(char *c, int size, std::string word)
{
	int count = 0;
	for (int i = 0; i < size; i+=16)
	{
		std::string s;
		for (int j = 0; isalpha(c[i+j]); ++j)
			s.push_back(c[i+j]);
		if (s == word) 
			++count;
	}
	return count; 
}

int main(int argc, char* argv[])
{
    int processId;
    int num_processes;
    int *to_return = NULL;
    double start_time, end_time;
	
 
    // Setup MPI
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &processId);
    MPI_Comm_size( MPI_COMM_WORLD, &num_processes);
 
    // Three arguments: <input file> <search word> <part B1 or part B2 to execute>
    if(argc != 4)
    {
        if(processId == 0)
        {
            std::cout << "ERROR: Incorrect number of arguments. Format is: <filename> <word> <b1/b2>" << std::endl;
        }
        MPI_Finalize();
        return 0; 
    }
	
	std::string word = argv[2];

    Lines lines;
	// Read the input file and put words into char array(lines)
    if (processId == 0) {
        std::ifstream file;
		file.imbue(std::locale(std::locale(), new letter_only()));
		file.open(argv[1]);
		std::string workString;
		int i = 0;
		while(file >> workString){
			memset(lines[i], '\0', 16);
			memcpy(lines[i++], workString.c_str(), workString.length());
		}
		
		for (i; i < ARRAY_SIZE; ++i)
			memset(lines[i], '\0', 16);
	}
	
//	***************** Add code as per your requirement below ***************** 
	
	start_time=MPI_Wtime();
	
	
	int chunkSize = ceil((float)ARRAY_SIZE / num_processes);
	int size = chunkSize * 16;
	char * c = new char[size];
	int localCount = 0;
	int totalCount = 0;
	
	MPI_Scatter(lines, size, MPI_CHAR, c, size, MPI_CHAR, 0, MPI_COMM_WORLD);
	localCount = countWords(c, size, word);
	
	if( std::string(argv[3]) == "b1" )
	{
		// Reduction for Part B1
		MPI_Reduce(&localCount, &totalCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	} 
	else 
	{
		// Point-To-Point communication for Part B2
		if (processId != 0)
		{
			int temp;
			MPI_Recv(&temp, 1, MPI_INT, processId-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			localCount += temp;
		}

		MPI_Send(&localCount, 1, MPI_INT, (processId+1)%num_processes, 0, MPI_COMM_WORLD);
		
		if (processId == 0)
			MPI_Recv(&totalCount, 1, MPI_INT, num_processes-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	
	
    if(processId == 0)
    {
        DoOutput(word, totalCount);
		
		end_time=MPI_Wtime();
        std::cout << "Time: " << ((double)end_time-start_time) << std::endl;
    }
 
 
	delete [] c;
    MPI_Finalize();
    return 0;
}