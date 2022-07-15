#include <climits>
#include <cstddef>
#include <vector>
#include <set>
#include <unordered_set>
#include <string>
#include <unitypes.h>

#ifndef SUMMARIZER_H
#define SUMMARIZER_H

typedef std::basic_string<uint8_t> uint8_string;

class Summarizer
{
	public:
		Summarizer(const char*, size_t); 
		void inputFilename(const char*);
		void printSummary();
	private:
		class SmartUTF8String /* Contains a raw char array, without a NUL termination byte. */
		{
			public:
				SmartUTF8String(size_t);
				uint8_t* getWriteableStringDoesNotUpdateStrlenOrCapacity() { return string; }
				size_t getCapacity() { return capacity; }
				size_t getStrlen() { return strlen; }
				size_t* giveCapacityGetStrlen();
				void reset(uint8_t*);
				~SmartUTF8String();
			private:
				// estimated maximum length of a POSIX filename transcoded to UTF-8, since UTF-8 characters are 1-4 bytes long
				static const size_t UTF8_FILENAME_MAX = std::FILENAME_MAX * 4;

				uint8_t* string; 
				size_t strlen; 
				size_t capacity;
				size_t* strlenPointer;
		};

		size_t greatestCommonChunkIndex;
		size_t patternIndex; 
		size_t rowLimit;
		const std::unordered_set<uint8_string> delimiters;
		std::vector<std::set<uint8_string>> pattern; 
		uint8_string scratch;
		const char* localeCode;
		SmartUTF8String first;
		SmartUTF8String second;

		void ingestString(char*);
		const uint8_t* getGraphemeClusterBreaks() { return first.getWriteableStringDoesNotUpdateStrlenOrCapacity(); }
		const uint8_t* getProcessedString() { return second.getWriteableStringDoesNotUpdateStrlenOrCapacity(); }
		size_t getProcessedStringLength() { return second.getStrlen(); }
		void checkResult(uint8_t*, SmartUTF8String&);
		void insertSubstringInNextColumn(const uint8_t*, size_t, size_t);
};

#endif /* SUMMARIZER_H */
