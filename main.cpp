/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* pattern: Summarizes the pattern of a list of filenames by slicing each one into     *
*          substrings, grouping together the Nth substrings from every filename, and  *
*          printing the unique substrings found in each of the N groups.              *
* Copyright (C) 2022  Joe Antaki  ->  joeantaki3 at gmail dot com                     *
*                                                                                     *
* This program is free software: you can redistribute it and/or modify                *
* it under the terms of the GNU General Public License as published by                *
* the Free Software Foundation, either version 3 of the License, or                   *
* (at your option) any later version.                                                 *
*                                                                                     *
* This program is distributed in the hope that it will be useful,                     *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                      *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                       *
* GNU General Public License for more details.                                        *
*                                                                                     *
* You should have received a copy of the GNU General Public License                   *
* along with this program.  If not, see <https://www.gnu.org/licenses/>.              *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
//TODO: better handling for multiple delimiters in a row/different numbers of columns in a filename; display "outlier"
// columns (columns that not all filenames have) in a way that makes them not confusing, no matter where they occur in
// the pattern
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include "summarizer.hpp"

const char* USAGE = "Usage: %s [OPTION]... DIRECTORY\n";

// called if the user supplies badly formed arguments to the program
void printUsageAndExit(char** argv)
{
	std::fprintf(stderr, USAGE, argv[0]);
	std::fprintf(stderr, "Or try '%s -h' for more information.\n", argv[0]);
	std::exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
	int option;
	char* delimiters = nullptr;
	while((option = getopt(argc, argv, "d:h")) != -1)
	{
		switch(option) 
		{
			case 'd': 
				delimiters = optarg;
				break;
            case 'h':
                std::printf(USAGE, argv[0]);
                std::puts("");
                std::puts("Summarize the pattern of the filenames in DIRECTORY:");
                std::puts("slice each filename into substrings, group together the Nth substrings from");
                std::puts("every filename, and print the unique substrings found in each of the N groups.");
                std::puts("");
                std::puts("  -d=DELIMITERS\tdivide the filenames into substrings split by the characters");
                std::puts("\t\tin DELIMITERS; each user-perceived character (a.k.a. grapheme");
                std::puts("\t\tcluster) in DELIMITERS is considered a separate delimiter");
                std::puts("  -h\t\tdisplay this help text and exit");
                std::puts("");
                std::puts("Without a DELIMITERS argument, each user-perceived character of every");
                std::puts("filename is its own substring by default.");
                std::puts("");
                std::puts("Example:");
                std::printf("  %s -d abc dir\tEach filename in \"dir\" will be split around\n", argv[0]);
                std::puts("\t\t\teach occurrence of 'a' or 'b' or 'c'.");

                std::exit(EXIT_SUCCESS);
                break;
			default:
				printUsageAndExit(argv);
		}
	}

    /*TODO: this returns an error code that must be checked
    #include <sys/ioctl.h>
	winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
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
		//TODO: handle stdin. Check for empty strings and don't process them
	}
	else
	{
		DIR* dir = opendir(argv[optind]);
		if(dir == nullptr)
		{
			std::perror(nullptr);
			std::exit(EXIT_FAILURE);
		}

        // read all filenames from the supplied directory, and feed them into the summarizer
		dirent* ent;
        char* filename;
		while((ent = readdir(dir)) != nullptr)
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
