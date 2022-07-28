#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include "summarizer.hpp"

const char* USAGE = "Usage: %s [OPTION]... DIRECTORY\n";

void printUsageAndExit(char** argv)
{
	std::fprintf(stderr, USAGE, argv[0]);
	std::fprintf(stderr, "Or try '%s -h' for more information.\n", argv[0]);
	std::exit(EXIT_FAILURE);
}

//TODO: check for empty strings in stdin input, and don't process them
int main(int argc, char* argv[])
{
	int option;
	char* delimiters = NULL;
	while((option = getopt(argc, argv, "rd:h")) != -1)
	{
		switch(option) 
		{
			case 'r':
				//TODO
				break;
			case 'd': 
				delimiters = optarg;
				break;
            case 'h':
                std::printf(USAGE, argv[0]);
                std::puts("");
                std::puts("Print the pattern of the filenames in DIRECTORY.");
                std::puts("A pattern is a sequence of columns, where each column");
                std::puts("is a set of unique substrings of filenames.");
                std::puts("");
                std::puts("  -d=DELIMITERS\tdivide the pattern into columns split by the characters");
                std::puts("\t\tin DELIMITERS; each user-perceived character (a.k.a. grapheme");
                std::puts("\t\tcluster) in DELIMITERS is considered a separate delimiter");
                std::puts("  -h\t\tdisplay this help text and exit");
                std::puts("");
                std::puts("Without a DELIMITERS argument, each character in the pattern");
                std::puts("is placed in its own column by default.");
                std::puts("");
                std::puts("Example:");
                std::printf("  %s -d abc dir\tThe pattern of the filenames in \"dir\" has a \n", argv[0]);
                std::puts("\t\t\tnew column on each occurrence of 'a' or 'b' or 'c'.");

                std::exit(EXIT_SUCCESS);
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
        char* filename;
		while((ent = readdir(dir)) != NULL)
		{
            filename = ent->d_name;
            if(std::strcmp(".", filename) && std::strcmp("..", filename)) //ignore the "." and ".." in the directory
            {
                summarizer.inputFilename(filename);
            }
		}

		closedir(dir);
	}

	summarizer.printSummary();
}
