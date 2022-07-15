#include <clocale>
#include <cstdlib>
#include <cstdio>
#include <climits>
#include <uniconv.h>
#include <uninorm.h>
#include <unigbrk.h>
#include "summarizer.hpp"

Summarizer::Summarizer(const char* _delimiters, size_t rowLimit) :
			greatestCommonChunkIndex(std::SIZE_MAX),
			patternIndex(0),
			rowLimit(rowLimit)
{
	std::setlocale(LC_ALL, "");
        localeCode = locale_charset();

	//TODO: maybe make this some method that's also called in inputFilename
	ingestString(_delimiters);
	const uint8_t* graphemeBreaks = getGraphemeClusterBreaks();
        const uint8_t* processedDelimiters = getProcessedString();
        size_t processedDelimitersLength = getProcessedStringLength();

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
	ingestString(filename);
	const uint8_t* graphemeBreaks = getGraphemeClusterBreaks();
	const uint8_t* processedFilename = getProcessedString();
	size_t processedFilenameLength = getProcessedStringLength();

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

void Summarizer::printSummary()
{

}

void Summarizer::ingestString(const char* filename)
{
	uint8_t* result;

	result = u8_conv_from_encoding(localeCode, iconveh_question_mark, filename, strlen(filename), NULL, first.getWriteableStringDoesNotUpdateStrlenOrCapacity(), first.giveCapacityGetStrlen());
	checkResult(result, first);

	result = u8_normalize(UNINORM_NFC, first.getWriteableStringDoesNotUpdateStrlenOrCapacity(), first.getStrlen(), second.getWriteableStringDoesNotUpdateStrlenOrCapacity(), second.giveCapacityGetStrlen());
	checkResult(result, second);

	size_t secondStrlen = second.getStrlen();
	*(first.giveCapacityGetStrlen()) = secondStrlen;
	if(first.getCapacity() < secondStrlen)
	{
		result = std::malloc(sizeof(*(first.getWriteableStringDoesNotUpdateStrlenOrCapacity())) * secondStrlen);
		checkResult(result, first);
	}
	u8_grapheme_breaks(second.getWriteableStringDoesNotUpdateStrlenOrCapacity(), secondStrlen, (char*) first.getWriteableStringDoesNotUpdateStrlenOrCapacity()); 
}

void Summarizer::checkResult(uint8_t* result, SmartUTF8String& smartString)
{
	if(result != smartString.getWriteableStringDoesNotUpdateStrlenOrCapacity())
	{
		//TODO: add filename logging for non-malloc errors, so can print "these filenames could not be processed:" at end
		if(ptr == NULL)
		{
			std::perror(NULL);
			std::exit(EXIT_FAILURE);
		}

		smartString.reset(result);
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

Summarizer::SmartUTF8String::SmartUTF8String() :
	strlen(0),
	capacity(UTF8_FILENAME_MAX),
	strlenPointer(&strlen)

{
	string = (uint8_t*) std::malloc(sizeof(*string) * capacity);
	if(ptr == NULL)
        {
                std::perror(NULL);
                std::exit(EXIT_FAILURE);
        }
}

size_t* Summarizer::SmartUTF8String::giveCapacityGetStrlen()
{
	strlen = capacity;
	return strlenPointer;
}

void Summarizer::SmartUTF8String::reset(uint8_t* other)
{
	std::free(string);
	string = other;
	capacity = strlen;
}

Summarizer::SmartUTF8String::~SmartUTF8String()
{
	std::free(string);	
}
