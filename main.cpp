#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include "summarizer.hpp"

const char* UTF8 = "UTF-8";

//TODO: unsync iostream with underlying stuff so its faster, or use (f)printf
void printErrorAndExit(char** argv)
{
	std::cerr << "Usage: " << argv[0] << " [-d delimiters] [-r] directory" << std::endl;
	std::cerr << "Or try '" << argv[0] << " -h' for more information." << std::endl; 
	exit(EXIT_FAILURE);
}

//TODO: check for empty strings in stdin input, and don't process them
int main(int argc, char* argv[])
{
	int option;
	while((option = getopt(argc, argv, "rd:")) != -1) 
	{
		switch(option) 
		{
			case 'r':
				break;
			case 'd': 
				//TODO: pointer to argument in optarg
				break;
			default:
				printErrorAndExit(argv);
		}
	}

	if(optind >= argc)
	{
		// no directory given as an argument, so check for a list of filenames from stdin
		if(isatty(STDIN_FILENO))
		{
			printErrorAndExit(argv);
		}
		//TODO: handle stdin
	}
}
