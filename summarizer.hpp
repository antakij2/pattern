#include <climits>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <set>
#include <unordered_set>
#include <string>
#include <unitypes.h>

#ifndef SUMMARIZER_H
#define SUMMARIZER_H

class Summarizer
{
	public:
		Summarizer(const char*, int);
		void inputFilename(const char*);
		void printSummary();
	private:
		typedef std::basic_string<uint8_t> uint8_string;

		class SmartUTF8Buffer /* Contains a raw char array, without a NUL termination byte. */
		{
			public:
				SmartUTF8Buffer(std::size_t);
				uint8_t* getWriteableStringDoesNotUpdateStrlenOrCapacity() { return string; }
				std::size_t getCapacity() { return capacity; }
				std::size_t getStrlen() { return strlen; }
				std::size_t* giveCapacityGetStrlen();
				void reset(const uint8_t*);
				~SmartUTF8Buffer();
			private:
				// estimated maximum length of a POSIX filename transcoded to UTF-8, since UTF-8 characters are 1-4 bytes long
				static const std::size_t UTF8_FILENAME_MAX = FILENAME_MAX * 4;

				uint8_t* string; 
				std::size_t strlen; 
				std::size_t capacity;
				std::size_t* strlenPointer;
		};

		struct GraphemeClusterString
		{
			GraphemeClusterString(const char*, std::size_t, std::size_t); 

			const uint8_string string;
			const std::size_t graphemeClusterCount;
		};

		static constexpr auto graphemeClusterStringComp = [](const GraphemeClusterString& lhs, const GraphemeClusterString& rhs) { return lhs.string < rhs.string; };

		std::size_t greatestCommonChunkIndex;
		std::size_t patternIndex; 
		int colLimit;
		const std::unordered_set<uint8_string> delimiters;
		std::vector<std::set<GraphemeClusterString, decltype(graphemeClusterStringComp)>> pattern; 
		std::vector<std::size_t> highestGraphemeClusterCounts;
		uint8_string scratch;
		const char* localeCode;
		SmartUTF8Buffer first; //FIXME: rename to be more descriptive, also "first"
		SmartUTF8Buffer second; // is shadowed by some local variables in methods

		void ingestString(char*);
		const uint8_t* getGraphemeClusterBreaks() { return first.getWriteableStringDoesNotUpdateStrlenOrCapacity(); } //TODO: get rid of these, just access "first" and "second" directly. During printSummary, they 
		const uint8_t* getProcessedString() { return second.getWriteableStringDoesNotUpdateStrlenOrCapacity(); }      // are not grapheme cluster breaks + the corresponding processed string 
		std::size_t getProcessedStringLength() { return second.getStrlen(); }					      // anymore
		void checkResult(const uint8_t*, SmartUTF8Buffer&);
		void insertInNextColumn(const uint8_t*, std::size_t, std::size_t, std::size_t);
};

#endif /* SUMMARIZER_H */
