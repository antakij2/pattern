#include <clocale>
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

// If "fromcode" is not already UTF-8, convert "source" from "fromcode" to UTF-8. Store result in "result"
const uint8_t* convertEncodingIfNecessary(const char* fromcode, const char* source, size_t sourceLength, uint8_t** result, size_t* resultLength)
{
	if(!strcmp(fromcode, UTF8))
	{
		return (const uint8_t*)source;
	}

	uint8_t* newResult = u8_conv_from_encoding(fromcode, iconveh_escape_sequence, source, sourceLength, NULL, result, resultLength); //TODO: undocumented iconveh_replacement_character?
	if(newResult != result)
	{
		free(*result);
		*result = newResult;
}

//TODO: use u8_conv_from_encoding if source encoding isn't "UTF-8", and make resultbuf an existing buffer (gotta do strlen tho and feed that in too). \uxxx if untranslatable character?
int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");

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
