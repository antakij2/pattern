#include <cuchar>
#include <climits>
#include <cstddef>
#include <vector>
#include <set>
#include <unordered_set>
#include <string>

#ifndef SUMMARIZER_H
#define SUMMARIZER_H

class Summarizer
{
	public:
		Summarizer(std::unordered_set<char32_t>&); //TODO: in main, do wchars need to be explicitly cast to char32_t, or must I use c++14 to get implicit conversion?
		void inputFilename(std::u32string&);
		//TODO: function to supply output strings
	private:
		size_t greatestCommonChunkIndex;
		size_t patternIndex; 
		const std::unordered_set<char32_t> delimiters;
		std::vector<std::set<std::u32string>> pattern; //TODO: create custom comparison object for this

		void insertSubstringInNextColumn(const std::u32string&, size_t&, size_t&);

		class TranscoderNormalizer
		{
			public:
				TranscoderNormalizer(const char*);
				const char* decode_normalize(char*);
			private:
				// estimated maximum length of a POSIX filename transcoded to UTF-8, since UTF-8 characters are 1-4 bytes long
				static const size_t UTF8_FILENAME_MAX = FILENAME_MAX * 4; 

				const char* fromcode;
				SmartUTF8String first;
				SmartUTF8String second;

				void checkResult(uint8_t*, SmartUTF8String&);

				class SmartUTF8String
				{
					public:
						SmartUTF8String(size_t);
						~SmartUTF8String();
						uint8_t* getWritableString() { return string; } 
						size_t* giveCapacityGetStrlen();
						void reset(uint8_t*);
					private:
						uint8_t string[];
						size_t strlen; /* This length includes the NUL termination byte. */
						size_t capacity;
						size_t* strlenPointer;

						void throwIfError(void* ptr) { if(ptr == NULL) throw std::runtime_error("Error!"); }//FIXME: the error reported by unistring functions returning
																    // NULL can be something other than ENOMEM, so generalize
																    // this function, or make a new one, to handle the others
																    // (after final output, have a comma-delimlited list of
																    // "these filenames could not be processed:"
				};
		};
};

#endif /* SUMMARIZER_H */
