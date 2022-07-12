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

//TODO: make seperate headers for nested classes, give them private constructors, and put "friend StringIngester" in SmartUTF8String, and "friend Summarizer" in StringIngester, so they can be instantiated by those classes?

class Summarizer
{
	public:
		Summarizer(const char*); 
		void inputFilename(const char*);
		//TODO: function to supply output strings
	private:
		size_t greatestCommonChunkIndex;
		size_t patternIndex; 
		const std::unordered_set<uint8_string> delimiters;
		std::vector<std::set<uint8_string>> pattern; //TODO: create custom comparison object for this
		uint8_string scratch;

		void insertSubstringInNextColumn(const uint8_t*, size_t, size_t);

		class StringIngester
		{
			public:
				StringIngester();
				void ingestString(char*);
				const uint8_t* getGraphemeClusterBreaks() { return first.string; }
				const uint8_t* getProcessedString() { return second.string; }
				size_t getProcessedStringLength() { return second.getStrlen(); }
			private:
				// estimated maximum length of a POSIX filename transcoded to UTF-8, since UTF-8 characters are 1-4 bytes long
				static const size_t UTF8_FILENAME_MAX = FILENAME_MAX * 4; 

				const char* fromcode;

				void checkResult(uint8_t*, SmartUTF8String&);

				class SmartUTF8String /* Contains a raw char array, without a NUL termination byte. */
				{
					public:
						uint8_t string[]; 

						SmartUTF8String(size_t);
						~SmartUTF8String();
						size_t getCapacity() { return capacity; }
						size_t getStrlen() { return strlen; }
						size_t* giveCapacityGetStrlen();
						void reset(uint8_t*);
					private:
						size_t strlen; 
						size_t capacity;
						size_t* strlenPointer;

						void throwIfError(void* ptr) { if(ptr == NULL) throw std::runtime_error("Error!"); }//FIXME: the error reported by unistring functions returning
																    // NULL can be something other than ENOMEM, so generalize
																    // this function, or make a new one, to handle the others
																    // (after final output, have a comma-delimlited list of
																    // "these filenames could not be processed:"
				};

				SmartUTF8String first;
				SmartUTF8String second;
		};

		StringIngester stringIngester;
};

#endif /* SUMMARIZER_H */
