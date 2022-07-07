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
#include "summarizer.hpp"

Summarizer::Summarizer(std::unordered_set<char32_t>& _delimiters) :
			greatestCommonChunkIndex(SIZE_MAX),
			patternIndex(0),
			delimiters(std::move(_delimiters))
			{}

//TODO: create string out of each chunk examined, and check for membership in delimiters set
void Summarizer::inputFilename(std::u32string& filename)
{
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


Summarizer::TranscoderNormalizer::TranscoderNormalizer(const char* fromcode) :
	fromcode(fromcode),
	first(new char[UTF8_FILENAME_MAX]),
	second(new char[UTF8_FILENAME_MAX]) 
{}

const char* Summarizer::TranscoderNormalizer::transcode_normalize(const char* filename)
{
	uint8_t* result;

	result = u8_conv_from_encoding(fromcode, iconveh_escape_sequence, filename, strlen(filename), /*TODO: if comes from a std::string, use that property */ NULL, first.get(), 
}


Summarizer::TranscoderNormalizer::SmartUTF8String::SmartUTF8String(size_t _capacity) :
	strlen(0),
	capacity(_capacity)

{
	string = (uint8_t*) malloc(sizeof(*string) * _capacity);
	throwIfMallocFailed(string);
	string[0] = '\0';
}

void Summarizer::TranscoderNormalizer::SmartUTF8String::reset(uint8_t* other, size_t otherStrlen, size_t otherCapacity)
{
	free(string);
	string = other;
	strlen = otherStrlen;
	capacity = otherCapacity;
}

Summarizer::TranscoderNormalizer::SmartUTF8String::~SmartUTF8String()
{
	free(string);	
}
