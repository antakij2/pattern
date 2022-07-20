#include <clocale>
#include <cstdlib>
#include <cstdio>
#include <uniconv.h>
#include <uninorm.h>
#include <unigbrk.h>
#include "summarizer.hpp"

Summarizer::Summarizer(const char* _delimiters, int colLimit) :
			greatestCommonChunkIndex(std::SIZE_MAX),
			patternIndex(0),
			colLimit(colLimit)
{
	std::setlocale(LC_ALL, "");
        localeCode = locale_charset();

	if(_delimiters == NULL)
	{
		ingestString(_delimiters);
		const uint8_t* graphemeBreaks = getGraphemeClusterBreaks();
		const uint8_t* processedDelimiters = getProcessedString();
		std::size_t processedDelimitersLength = getProcessedStringLength();

		std::size_t first = 0;
		std::size_t last = 1;
		
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
}

void Summarizer::inputFilename(const char* filename)
{
	ingestString(filename);
	const uint8_t* graphemeBreaks = getGraphemeClusterBreaks();
	const uint8_t* processedFilename = getProcessedString();
	std::size_t processedFilenameLength = getProcessedStringLength();

	patternIndex = 0;
	std::size_t first = 0;
	std::size_t prev = 0;
	std::size_t last = 1;
	std::size_t graphemeClusterCount = -1;

	//TODO:
	// Either filename[last] is a delimiter, or there are no delimiters - meaning each character 
	// is considered separately. Either way, the substring denoted by [first, last) will 
	// be put into a column, and then filename[last] will be in the next column, by itself.
	for(; last < processedFilenameLength; ++last) 
	{
		if(graphemeBreaks[last])
		{
			++graphemeClusterCount;

			if(delimiters.empty() || delimiters.count(scratch.assign(processedFilename + prev, last - prev)))
			{
				if(first != prev) 
				{
					insertInNextColumn(processedFilename, first, prev, graphemeClusterCount);
				}
				insertInNextColumn(processedFilename, prev, last, 1);

				first = last;
				graphemeClusterCount = -1;
			}

			prev = last;
		}
	}

	insertInNextColumn(processedFilename, first, processedFilenameLength, 1);

	if(patternIndex < greatestCommonChunkIndex)
	{
		greatestCommonChunkIndex = patternIndex;
	}
}

void Summarizer::printSummary()
{
	// scan for the "tallest column" (largest set) in the pattern. This will dictate the amount of rows in the output
	const std::size_t patternSize = pattern.size();
	std::size_t tallestColumnSize = 0;

	for(std::size_t i = 0; i < patternSize; ++i)
	{
		/*TODO:
		std::size_t currSize = pattern[i].size();
		if(i > greatestCommonChunkIndex)
		{
			//behavior for adding blank space under chunks longer than smallest common chunk length
			++currSize;
		}
		*/

		if(pattern[i].size() > tallestColumnSize)
		{
			tallestColumnSize = pattern[i].size();
		}
	}
	
	std::vector<std::string> output(tallestColumnSize);

	// build up the pattern summary, one column at a time
	char* result;
	for(std::size_t i=0; i < patternSize; ++i)
	{
		auto chunkIterator = pattern[i].crbegin();
		auto chunkEnd = pattern[i].crend();
		for(auto outputRowIterator = output.rbegin(); outputRowIterator != output.rend(); ++outputRowIterator)
		{
			if(chunkIterator != chunkEnd)
			{
				result = u8_conv_to_encoding(localeCode, 
						iconveh_question_mark, 
						chunkIterator -> string.data(), 
						chunkIterator -> string.size(), 
						NULL, 
						(char*) first.getWriteableStringDoesNotUpdateStrlenOrCapacity(), //FIXME: these casts back and forth might be incorrect?
						first.giveCapacityGetStrlen());
				checkResult((uint8_t*) result, first); //TODO: technically not necessary?

				outputRowIterator -> append((char*) first.getWriteableStringDoesNotUpdateStrlenOrCapacity(), first.getStrlen());
				outputRowIterator -> append(' ', highestGraphemeClusterCounts[i] - (chunkIterator -> graphemeClusterCount));

				++chunkIterator;
			}
			else
			{
				outputRowIterator -> append(' ', highestGraphemeClusterCounts[i]);
			}

			if(i != patternSize - 1)
			{
				outputRowIterator -> push_back(' '); //column divider
			}
		}
	}

	// print out the pattern summary, keeping in mind the column limit of the user's terminal
	// FIXME: keep in mind the column limit of the user's terminal. ulc_grapheme_breaks to know where to break this text?
	std::printf("\n");
	for(auto it=output.begin(); it != output.end(); ++it)
	{
		printf("%s\n", it -> data());
	}
}

void Summarizer::ingestString(const char* filename)
{
	uint8_t* result;

	result = u8_conv_from_encoding(localeCode, iconveh_question_mark, filename, strlen(filename), NULL, first.getWriteableStringDoesNotUpdateStrlenOrCapacity(), first.giveCapacityGetStrlen());
	checkResult(result, first);

	result = u8_normalize(UNINORM_NFC, first.getWriteableStringDoesNotUpdateStrlenOrCapacity(), first.getStrlen(), second.getWriteableStringDoesNotUpdateStrlenOrCapacity(), second.giveCapacityGetStrlen());
	checkResult(result, second);

	std::size_t secondStrlen = second.getStrlen();
	*(first.giveCapacityGetStrlen()) = secondStrlen;
	if(first.getCapacity() < secondStrlen)
	{
		result = std::malloc(sizeof(*(first.getWriteableStringDoesNotUpdateStrlenOrCapacity())) * secondStrlen);
		checkResult(result, first);
	}
	u8_grapheme_breaks(second.getWriteableStringDoesNotUpdateStrlenOrCapacity(), secondStrlen, (char*) first.getWriteableStringDoesNotUpdateStrlenOrCapacity()); //FIXME: this (char*) cast is not guarenteed correct?
}

void Summarizer::checkResult(const uint8_t* result, SmartUTF8Buffer& smartString) 
{
	if(result != smartString.getWriteableStringDoesNotUpdateStrlenOrCapacity())
	{
		//TODO: add filename logging for non-malloc errors, so can print "these filenames could not be processed:" at end
		if(result == NULL)
		{
			std::perror(NULL);
			std::exit(EXIT_FAILURE);
		}

		smartString.reset(result);
	}
}

void Summarizer::insertInNextColumn(const uint8_t* str, std::size_t start, std::size_t end, std::size_t graphemeClusterCount)
{
	if(pattern.size() == patternIndex)
	{
		// create a new column
		pattern.emplace_back(graphemeClusterStringComp);
		highestGraphemeClusterCounts.push_back(0);
	}

	pattern[patternIndex].emplace(str + start, start - end, graphemeClusterCount);
	if(graphemeClusterCount > highestGraphemeClusterCounts[patternIndex])
	{
		highestGraphemeClusterCounts[patternIndex] = graphemeClusterCount;
	}

	++patternIndex;
}

Summarizer::SmartUTF8Buffer::SmartUTF8Buffer() :
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

std::size_t* Summarizer::SmartUTF8Buffer::giveCapacityGetStrlen()
{
	strlen = capacity;
	return strlenPointer;
}

void Summarizer::SmartUTF8Buffer::reset(const uint8_t* other)
{
	std::free(string);
	string = other;
	capacity = strlen;
}

Summarizer::SmartUTF8Buffer::~SmartUTF8Buffer()
{
	std::free(string);	
}

Summarizer::GraphemeClusterString::GraphemeClusterString(const char* s, std::size_t n, std::size_t graphemeClusterCount) :
	string(s, n),
	graphemeClusterCount(graphemeClusterCount)
{}
