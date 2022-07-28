#include <clocale>
#include <cstring>
#include <cstdio>
#include <uniconv.h>
#include <uninorm.h>
#include <unigbrk.h>
#include "summarizer.hpp"

Summarizer::Summarizer(const char* _delimiters, int colLimit) :
			greatestCommonChunkIndex(SIZE_MAX),
			patternIndex(0),
			colLimit(colLimit)
{
	std::setlocale(LC_ALL, "");
    localeCode = locale_charset();

	if(_delimiters != NULL)
	{
		ingestString(_delimiters);
		const char* graphemeBreaks = charBuffer.getWriteableStringDoesNotUpdateStringLengthOrCapacity();
		const uint8_t* processedDelimiters = utf8BufferOuter.getWriteableStringDoesNotUpdateStringLengthOrCapacity();
		std::size_t processedDelimitersLength = utf8BufferOuter.getStringLength();

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
	const char* graphemeBreaks = charBuffer.getWriteableStringDoesNotUpdateStringLengthOrCapacity();
	const uint8_t* processedFilename = utf8BufferOuter.getWriteableStringDoesNotUpdateStringLengthOrCapacity();
	std::size_t processedFilenameLength = utf8BufferOuter.getStringLength();

	patternIndex = 0;
	std::size_t first = 0;
	std::size_t prev = 0;
	std::size_t last = 1;
	std::size_t graphemeClusterCount = 0; //specifically, the number of grapheme clusters in [first, prev)

	for(; last < processedFilenameLength; ++last)
	{
		if(graphemeBreaks[last])
		{
			if(delimiters.empty() || delimiters.count(scratch.assign(processedFilename + prev, last - prev)))
			{
				if(first != prev)
				{
					insertInNextColumn(processedFilename, first, prev, graphemeClusterCount);
				}
				insertInNextColumn(processedFilename, prev, last, 1);

				first = last;
				graphemeClusterCount = SIZE_MAX; // intentionally causing overflow on next increment of graphemeClusterCount
			}

			prev = last;
            ++graphemeClusterCount;
        }
	}

    if(first == prev)
    {
        // all characters preceding "prev" have already been processed, so just take care of what's in [prev, last)
        insertInNextColumn(processedFilename, prev, last, 1);
    }
    else
    {
        // "delimiters" is not empty, and no characters in [first, prev) are delimiters

        if(delimiters.count(scratch.assign(processedFilename + prev, last - prev)))
        {
            insertInNextColumn(processedFilename, first, prev, graphemeClusterCount);
            insertInNextColumn(processedFilename, prev, last, 1);
        }
        else
        {
            insertInNextColumn(processedFilename, first, last, graphemeClusterCount + 1);
        }
    }

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
                                             charBuffer.getWriteableStringDoesNotUpdateStringLengthOrCapacity(),
                                             charBuffer.giveCapacityGetStringLength());
				checkResult(result, charBuffer); //TODO: technically not necessary?

				outputRowIterator -> append(charBuffer.getWriteableStringDoesNotUpdateStringLengthOrCapacity(),
                                            charBuffer.getStringLength());
				outputRowIterator -> append(highestGraphemeClusterCounts[i] - (chunkIterator -> graphemeClusterCount), ' ');

                ++chunkIterator;
			}
			else
			{
				outputRowIterator -> append(highestGraphemeClusterCounts[i], ' ');
			}

			if(i != patternSize - 1)
			{
				outputRowIterator -> push_back(' '); //column divider
			}
		}
	}

	// print out the pattern summary, keeping in mind the column limit of the user's terminal
	// TODO: keep in mind the column limit of the user's terminal. ulc_grapheme_breaks to know where to break this text?
	for(auto it=output.begin(); it != output.end(); ++it)
	{
		printf("%s\n", it -> data());
	}
}

void Summarizer::ingestString(const char* filename)
{
	uint8_t* result;

	result = u8_conv_from_encoding(localeCode, iconveh_question_mark, filename, std::strlen(filename), NULL,
                                   _utf8BufferInner.getWriteableStringDoesNotUpdateStringLengthOrCapacity(),
                                   _utf8BufferInner.giveCapacityGetStringLength());
	checkResult(result, _utf8BufferInner);

	result = u8_normalize(UNINORM_NFC, _utf8BufferInner.getWriteableStringDoesNotUpdateStringLengthOrCapacity(), _utf8BufferInner.getStringLength(),
                          utf8BufferOuter.getWriteableStringDoesNotUpdateStringLengthOrCapacity(),
                          utf8BufferOuter.giveCapacityGetStringLength());
	checkResult(result, utf8BufferOuter);

    // manually make sure that the char buffer into which the grapheme breaks will be recorded is big enough
    std::size_t outerStringLength = utf8BufferOuter.getStringLength();
	*(_utf8BufferInner.giveCapacityGetStringLength()) = outerStringLength;
	if(_utf8BufferInner.getCapacity() < outerStringLength)
	{
		char* graphemeBreaksResult = (char*) std::malloc(sizeof(*(charBuffer.getWriteableStringDoesNotUpdateStringLengthOrCapacity())) * outerStringLength);
		checkResult(graphemeBreaksResult, charBuffer);
	}
	u8_grapheme_breaks(utf8BufferOuter.getWriteableStringDoesNotUpdateStringLengthOrCapacity(), outerStringLength, charBuffer.getWriteableStringDoesNotUpdateStringLengthOrCapacity());
}

template <class T>
void Summarizer::checkResult(T* result, SmartBuffer<T>& smartString)
{
	if(result != smartString.getWriteableStringDoesNotUpdateStringLengthOrCapacity())
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

	pattern[patternIndex].emplace(str + start, end - start, graphemeClusterCount);
	if(graphemeClusterCount > highestGraphemeClusterCounts[patternIndex])
	{
		highestGraphemeClusterCounts[patternIndex] = graphemeClusterCount;
	}

	++patternIndex;
}

template <class T>
Summarizer::SmartBuffer<T>::SmartBuffer() :
        stringLength(0),
        capacity(UTF8_FILENAME_MAX),
        stringLengthPointer(&stringLength)

{
	string = (T*) std::malloc(sizeof(*string) * capacity);
	if(string == NULL)
    {
        std::perror(NULL);
        std::exit(EXIT_FAILURE);
    }
}

template <class T>
std::size_t* Summarizer::SmartBuffer<T>::giveCapacityGetStringLength()
{
    stringLength = capacity;
	return stringLengthPointer;
}

template <class T>
void Summarizer::SmartBuffer<T>::reset(T* other)
{
	std::free(string);
	string = other;
	capacity = stringLength;
}

Summarizer::GraphemeClusterString::GraphemeClusterString(const uint8_t* s, std::size_t n, std::size_t graphemeClusterCount) :
	string(s, n),
	graphemeClusterCount(graphemeClusterCount)
{}
