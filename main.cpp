#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <dirent.h>
#include "summarizer.hpp"

void printUsageAndExit(char** argv)
{
	std::fprintf(stderr, "Usage: %s [-d delimiters] [-r] directory\n", argv[0]);
	std::fprintf(stderr, "Or try '%s -h' for more information.\n", argv[0]); 
	std::exit(EXIT_FAILURE);
}

//TODO: check for empty strings in stdin input, and don't process them
int main(int argc, char* argv[])
{
	int option;
	char* delimiters = NULL;
	while((option = getopt(argc, argv, "rd:")) != -1) 
	{
		switch(option) 
		{
			case 'r':
				//TODO
				break;
			case 'd': 
				delimiters = optarg; //TODO: is this safe if the arguments get permuted?
				break;
			default:
				printUsageAndExit(argv);
		}
	}

    /*TODO:
    #include <sys/ioctl.h>
	winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); //TODO: this returns an error code that must be checked
    (w.ws_col)
    */

	Summarizer summarizer(delimiters);
	if(optind >= argc)
	{
		// no directory given as an argument, so check for a list of filenames from stdin
		if(isatty(STDIN_FILENO))
		{
			printUsageAndExit(argv);
		}
		//TODO: handle stdin
	}
	else
	{
		DIR* dir = opendir(argv[optind]);
		if(dir == NULL)
		{
			std::perror(NULL);
			std::exit(EXIT_FAILURE);
		}

		dirent* ent;
		while((ent = readdir(dir)) != NULL)
		{
            //TODO: ignore '.' and '..' in directory
			summarizer.inputFilename(ent -> d_name);
		}

		closedir(dir);
	}

	summarizer.printSummary();
}
