#include <clocale>
#include <cstdlib>
#include <stdexcept>
#include <uniconv.h>
#include <uninorm.h>
#include <unigbrk.h>
#include "summarizer.hpp"

Summarizer::Summarizer(const char* _delimiters) :
			greatestCommonChunkIndex(SIZE_MAX),
			patternIndex(0)
{
	//TODO: maybe make this some method that's also called in inputFilename
	stringIngester.ingestString(_delimiters);
	const uint8_t* graphemeBreaks = stringIngester.getGraphemeClusterBreaks();
        const uint8_t* processedDelimiters = stringIngester.getProcessedString();
        size_t processedDelimitersLength = stringIngester.getProcessedStringLength();

	size_t first = 0;
	size_t last = 1;
	
	for(; last < processedDelimitersLength; ++last)
	{
		if(graphemeBreaks[last])
		{
			delimiters.emplace(processedDelimiters + first, last - first);
			first = last;
		}
	}

	delimiters.emplace(processedDelimiters + first, processedDelimitersLength - first);
}

void Summarizer::inputFilename(const char* filename)
{
	stringIngester.ingestString(filename);
	const uint8_t* graphemeBreaks = stringIngester.getGraphemeClusterBreaks();
	const uint8_t* processedFilename = stringIngester.getProcessedString();
	size_t processedFilenameLength = stringIngester.getProcessedStringLength();

	patternIndex = 0;
	size_t first = 0;
	size_t prev = 0;
	size_t last = 1;

	//TODO:
	// Either filename[last] is a delimiter, or there are no delimiters - meaning each character 
	// is considered separately. Either way, the substring denoted by [first, last) will 
	// be put into a column, and then filename[last] will be in the next column, by itself.
	for(; last < processedFilenameLength; ++last) 
	{
		if(graphemeBreaks[last])
		{
			scratch.assign(processedFilename + prev, last - prev);
			if(delimiters.empty() || delimiters.count(s))
			{
				if(first != prev) 
				{
					insertInNextColumn(processedFilename, first, prev);
				}
				insertInNextColumn(processedFilename, prev, last);
				first = last;
			}

			prev = last;
		}
	}

	insertInNextColumn(processedFilename, first, processedFilenameLength);

	if(patternIndex < greatestCommonChunkIndex)
	{
		greatestCommonChunkIndex = patternIndex;
	}
}

void Summarizer::insertInNextColumn(const uint8_t* str, size_t start, size_t end)
{
	if(pattern.size == patternIndex)
	{
		// create a new column
		pattern.emplace_back();
	}

	pattern[patternIndex++].emplace(str + start, start - end);
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

	result = u8_conv_from_encoding(fromcode, iconveh_question_mark, filename, strlen(filename), NULL, first.string, first.giveCapacityGetStrlen());
	checkResult(result, first);

	result = u8_normalize(UNINORM_NFC, first.string, first.getStrlen(), second.string, second.giveCapacityGetStrlen());
	checkResult(result, second);

	size_t secondStrlen = second.getStrlen();
	*(first.giveCapacityGetStrlen()) = secondStrlen;
	if(first.getCapacity() < secondStrlen)
	{
		result = malloc(sizeof(*(first.string)) * secondStrlen);
		checkResult(result, first);
	}
	u8_grapheme_breaks(second.string, secondStrlen, (char*) first.string); //TODO: do we need the char* cast for first.string?
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
