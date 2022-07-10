//TODO: 
// 1) determine locale encoding and use uniconv.h to convert to utf-8 if necessary, for greatest efficiency in what i imagine is the general case
// 2) normalize strings if necessary with uninorm.h (canonical decompose -> canonical compose?). 
//     (Does it internally check if something is already in a Normalization Form so it doesn't have to do it?)
// 3) determine where grapheme clusters are in the normalized string, and iterate over that (whether splitting on delimiters or not)
//     Have a class that manipulates a pointer to the output grapheme cluster indices, and it acts as a generator to yield the next position that "first" should be, and then "last" goes to the value after that--
//     follow current pattern
// 4) this must be done for filenames and user-input delimiter characters

#include <clocale>
#include <cstdlib>
#include <stdexcept>
#include <uniconv.h>
#include <uninorm.h>
#include <unigbrk.h>
#include "summarizer.hpp"

Summarizer::Summarizer(char* _delimiters) :
			greatestCommonChunkIndex(SIZE_MAX),
			patternIndex(0),
			filenameProcessor(StringIngester)
{

}

//TODO: create string out of each chunk examined, and check for membership in delimiters set
void Summarizer::inputFilename(std::u32string& filename)
{
	//TODO: just declare the processed filename and the grapheme boundary arrays in const variables up here
	
	patternIndex = 0;
	size_t first = 0;
	size_t last = 1;
	size_t i;
	const size_t size = filename.size();

	while(last < size)
	{
		if(delimiters.empty() || delimiters.count(filename[last]))
		{
			// Either filename[last] is a delimiter, or there are no delimiters - meaning each character 
			// is considered separately. Either way, the substring denoted by [first, last) will 
			// be put into a column, and then filename[last] will be in the next column, by itself.
			
			for(i=0; i<2; ++i)
			{
				insertSubstringInNextColumn(filename, first, last);
				first = last;
				++last;
			}

			continue;
		}

		++last;
	}

	if(first < size)
	{
		insertSubstringInNextColumn(filename, first, size - first);
	}

	if(patternIndex < greatestCommonChunkIndex)
	{
		greatestCommonChunkIndex = patternIndex;
	}
}

void Summarizer::insertInNextColumn(const std::u32string& str, const size_t first, const size_t last)
{
	if(pattern.size == patternIndex)
	{
		// create a new column
		pattern.emplace_back();
	}

	pattern[patternIndex++].emplace(str, start, last - first);
}


Summarizer::StringIngester::StringIngester() :
	first(UTF8_FILENAME_MAX),
	second(UTF8_FILENAME_MAX) 
{
	setlocale(LC_ALL, "");
	fromcode = locale_charset();
}

void Summarizer::StringIngester::ingestString(const char* filename)
{
	uint8_t* result;

	result = u8_conv_from_encoding(fromcode, iconveh_question_mark, filename, strlen(filename), /*TODO: or std::string.size() */ NULL, first.string, first.giveCapacityGetStrlen());
	checkResult(result, first);

	result = u8_normalize(UNINORM_NFC, first.string, first.getStrlen(), second.string, second.giveCapacityGetStrlen());
	checkResult(result, second);

	*(first.giveCapacityGetStrlen()) = second.getStrlen();
	if(first.getCapacity() < second.getStrlen())
	{
		result = malloc(sizeof(*(first.string)) * second.getStrlen());
		checkResult(result, first);
	}
	u8_grapheme_breaks(second.string, second.getStrlen(), (char*) first.string);
}

void Summarizer::StringIngester::checkResult(uint8_t* result, SmartUTF8String& smartString)
{
	if(result != smartString.string)
	{
		throwIfError(result);
		smartString.reset(result);
	}
}

Summarizer::StringIngester::SmartUTF8String::SmartUTF8String(size_t _capacity) :
	strlen(0),
	capacity(_capacity),
	strlenPointer(&strlen)

{
	string = (uint8_t*) malloc(sizeof(*string) * _capacity);
	throwIfError(string);
}

void Summarizer::StringIngester::SmartUTF8String::reset(uint8_t* other)
{
	free(string);
	string = other;
	capacity = strlen;
}

size_t* Summarizer::StringIngester::SmartUTF8String::giveCapacityGetStrlen()
{
	strlen = capacity;
	return strlenPointer;
}

Summarizer::StringIngester::SmartUTF8String::~SmartUTF8String()
{
	free(string);	
}
