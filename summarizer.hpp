/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* pattern: Summarizes the pattern in a group of filenames by slicing each one into    *
*          substrings, grouping together the Nth substrings from every filename, and  *
*          printing the unique substrings found in each of the N groups.              *
* Copyright (C) 2022  Joe Antaki  ->  joeantaki3 at gmail dot com                     *
*                                                                                     *
* This program is free software: you can redistribute it and/or modify                *
* it under the terms of the GNU General Public License as published by                *
* the Free Software Foundation, either version 3 of the License, or                   *
* (at your option) any later version.                                                 *
*                                                                                     *
* This program is distributed in the hope that it will be useful,                     *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                      *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                       *
* GNU General Public License for more details.                                        *
*                                                                                     *
* You should have received a copy of the GNU General Public License                   *
* along with this program.  If not, see <https://www.gnu.org/licenses/>.              *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <set>
#include <string>
#include <unitypes.h>

#ifndef SUMMARIZER_H
#define SUMMARIZER_H

/* Only the public methods of class Summarizer are intended to be public API for any client code. */

/*
 * Computes and prints the pattern of a list of filenames. Takes into account the user's locale, respects
 * user-perceived characters (a.k.a. grapheme clusters) as defined by https://unicode.org/reports/tr29/ , and
 * prior to comparing characters, performs text normalization as defined by https://unicode.org/reports/tr15/ .
 */
class Summarizer
{
	public:
        /*
         * Constructs a Summarizer object.
         * @param _delimiters the user-perceived characters around which filenames will be split. If _delimiters
         *        contains multiple user-perceived characters e.g. "ab", then 'a' and 'b' are separate delimiters;
         *        the string "ab" is not a delimiter. Can be NULL to indicate no delimiters in particular, in which
         *        case the filenames are all broken down into individual user-perceived characters
         */
		Summarizer(const char* _delimiters);

        /*
         * Converts the supplied filename into utf-8, normalizes it, and splits it into substrings in accordance with
         * the user-determined delimiters. Adds these substrings into the running computation of the overall pattern
         * of filenames.
         * @param filename the filename to be ingested into the pattern
         */
		void inputFilename(const char* filename);

        /*
         * Prints the pattern of all the filenames that have been supplied. The Nth substrings of every filename are
         * put together in a group, resulting in N groups. Then the unique substrings within each group are
         * printed in a vertical column, with all N columns appearing side-by-side on the user's terminal.
         */
		void printSummary();
	private:
		typedef std::basic_string<uint8_t> uint8_string;

        /*
         * Class for interacting with the libunistring C library.
         * Contains a resizable character buffer, without a NUL termination byte.
         * @tparam T the type of character in the buffer. Meant to be either char or uint8_t
         */
        template <class T>
		class SmartBuffer
		{
			public:
                /*
                 * Construct a SmartBuffer object. Allocates a character buffer with a size of UTF8_FILENAME_MAX.
                 * Halts program if there is an error in buffer allocation.
                 */
				SmartBuffer();

                /*
                 * (For use alongside SmartBuffer::giveCapacityGetStringLength and Summarizer::checkResult).
                 * Returns a pointer to the non-const inner buffer, which may be written to by a libunistring function.
                 * But does NOT update the SmartBuffer object's stringLength or capacity variables to agree with any
                 * newly written data in the buffer.
                 * @return a pointer to the non-const inner buffer
                 */
				T* getWriteableStringDoesNotUpdateStringLengthOrCapacity() { return string; }

                /*
                 * (For use alongside SmartBuffer::getWriteableStringDoesNotUpdateStringLengthOrCapacity and Summarizer::checkResult).
                 * Copies the buffer's capacity value to the variable holding the length of the string in the buffer,
                 * stringLength. Then returns a pointer to the non-const stringLength, in the expectation that a new
                 * string will be written to the buffer, and the length of that string will be written back to stringLength
                 * by a libunistring function, via the pointer.
                 * @return a pointer to the non-const stringLength, which, as of this method call, temporarily holds the
                 *         buffer's capacity value instead of the length of the buffer's string
                 */
				std::size_t* giveCapacityGetStringLength();

                /*
                 * (To be called by SmartBuffer::checkResult)
                 * Frees the current inner buffer and sets "other" to be the new inner buffer. It is assumed that
                 * "other" was allocated by a libunistring function, and is only as long as the string inside it.
                 * Therefore, the value of the SmartBuffer's stringLength variable (assumed to hold the length of
                 * the string in "other") is also copied over to the SmartBuffer's capacity variable.
                 * @param other the new character buffer to set as the SmartBuffer object's inner buffer
                 */
				void reset(T* other);

                /*
                 * Frees the inner buffer.
                 */
				~SmartBuffer() { std::free(string); }

                std::size_t getCapacity() { return capacity; }
                std::size_t getStringLength() { return stringLength; }
			private:
                /* estimated maximum length of a POSIX filename transcoded to UTF-8, since UTF-8 characters are 1-4 bytes long */
				static const std::size_t UTF8_FILENAME_MAX = FILENAME_MAX * 4;

				T* string; /* the inner character buffer */
				std::size_t stringLength; /* the length of the string in the buffer */
                std::size_t capacity; /* the maximum capacity of the buffer */
				std::size_t* stringLengthPointer; /* pointer to stringLength. Used as an in/out parameter */
		};

        /*
         * Holds a std::string of utf-8 characters, along with the number of columns that string takes up on a terminal
         * screen (assuming a monospace font).
         */
		struct DisplayWidthString
		{
            /*
             * Construct a DisplayWidthString object.
             * @param s the source buffer of uint8_t from which to construct the std::string
             * @param n the number of uint8_t characters to include in the std::string, starting from the position
             *        pointed to by s
             * @param width the number of columns required to display the string on a terminal
             */
			DisplayWidthString(const uint8_t* s, std::size_t n, int width);

			const uint8_string string;
			const int width;
		};

        /*
         * A comparison function for objects of type DisplayWidthString. Compares the std::strings held by each object.
         * @param lhs the first DisplayWidthString object
         * @param rhs the second DisplayWidthString object
         * @return true if lhs's std::string comes lexicographically before rhs's std::string; false otherwise.
         */
        static bool displayWidthStringComp (const DisplayWidthString& lhs, const DisplayWidthString& rhs)
        {
            return lhs.string < rhs.string;
        }

        /* tracks the largest common index of substrings ("chunk") that get inserted into the pattern, over all filenames */
		std::size_t greatestCommonChunkIndex;

        /* keeps track of the next set in the pattern to insert filename substrings into, as a filename is being ingested */
        std::size_t patternIndex;

        /* sequence of N sets, each one holding the unique Nth substrings of all ingested filenames */
		std::vector<std::set<DisplayWidthString, decltype(displayWidthStringComp)*>> pattern; //TODO: make std::forward_list

        /* tracks the highest display width of any substring in a set, for all sets in pattern.
           Used to pad a set's substrings with spaces when printing out the pattern */
		std::vector<int> highestWidths;

        int colLimit; /* the number of columns in the user's terminal */
        std::set<uint8_string> delimiters; /* normalized, utf-8 encoded version of the delimiters passed into the Summarizer object*/
        uint8_string scratch; /* extra std::string used in determining when a delimiter is encountered in filenames */
		const char* localeCode; /* libunistring-recognized code for the user's locale */
		SmartBuffer<uint8_t> _utf8BufferInner; /* scratch buffer used when ingesting filenames */
		SmartBuffer<uint8_t> utf8BufferOuter; /* buffer that contains utf-8 encoded, normalized filenames */
		SmartBuffer<char> charBuffer; /* used to locate grapheme clusters in filenames, and to encode the pattern's substrings in the user's locale */

        /*
         * Helper function for Summarizer::ingestFilename.
         * Calls the libunistring functions to transcode and normalize the filename.
         * @param filename the filename to be ingested into the pattern
         */
		void ingestString(const char* filename);

        /*
         * (For use alongside SmartBuffer::getWriteableStringDoesNotUpdateStringLengthOrCapacity and
         * SmartBuffer::giveCapacityGetStringLength).
         * Checks whether result is the same buffer as the one contained by smartString.
         * If it is not, set smartString to contain result instead of its current buffer.
         * Halts program if result is NULL, indicating an error.
         * @param result buffer containing new data written by libunistring
         * @param smartString object which should wrap result
         */
		template <class T> void checkResult(T* result, SmartBuffer<T>& smartString);

        /*
         * Adds the next substring of the currently-being-ingested filename to the next set in the pattern, and
         * records the string's display width.
         * @param str the character buffer to take a substring of
         * @param start where the substring in str begins (inclusive)
         * @param end where the substring in str ends (exclusive)
         */
		void insertInNextColumn(const uint8_t* str, std::size_t start, std::size_t end);
};

#endif /* SUMMARIZER_H */
