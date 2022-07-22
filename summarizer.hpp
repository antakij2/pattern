#include <climits>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <set>
#include <string>
#include <unitypes.h>

#ifndef SUMMARIZER_H
#define SUMMARIZER_H

class Summarizer
{
	public:
		Summarizer(const char*, std::size_t);
		void inputFilename(const char*);
		void printSummary();
	private:
		typedef std::basic_string<uint8_t> uint8_string;

		template <class T>
		class SmartBuffer /* Contains a raw character array, without a NUL termination byte. */
		{
			public:
				SmartBuffer();
				T* getWriteableStringDoesNotUpdateStringLengthOrCapacity() { return string; }
				std::size_t getCapacity() { return capacity; }
				std::size_t getStringLength() { return stringLength; }
				std::size_t* giveCapacityGetStringLength();
				void reset(T*);
				~SmartBuffer();
			private:
				// estimated maximum length of a POSIX filename transcoded to UTF-8, since UTF-8 characters are 1-4 bytes long
				static const std::size_t UTF8_FILENAME_MAX = FILENAME_MAX * 4;

				T* string;
				std::size_t stringLength;
				std::size_t capacity;
				std::size_t* stringLengthPointer;
		};

		struct GraphemeClusterString
		{
			GraphemeClusterString(const uint8_t*, std::size_t, std::size_t);

			const uint8_string string;
			const std::size_t graphemeClusterCount;
		};

        static bool graphemeClusterStringComp (const GraphemeClusterString& lhs, const GraphemeClusterString& rhs) { return lhs.string < rhs.string; }

		std::size_t greatestCommonChunkIndex;
		std::size_t patternIndex; 
		std::size_t colLimit;
		std::set<uint8_string> delimiters;
		std::vector<std::set<GraphemeClusterString, decltype(graphemeClusterStringComp)*>> pattern;
		std::vector<std::size_t> highestGraphemeClusterCounts;
		uint8_string scratch;
		const char* localeCode;
		SmartBuffer<uint8_t> _utf8BufferInner;
		SmartBuffer<uint8_t> utf8BufferOuter;
		SmartBuffer<char> charBuffer;

		void ingestString(const char*);
		template <class T> void checkResult(T*, SmartBuffer<T>&);
		void insertInNextColumn(const uint8_t*, std::size_t, std::size_t, std::size_t);
};

#endif /* SUMMARIZER_H */
