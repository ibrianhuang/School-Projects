#include <iostream>
#include <string>
#include "HashMap.hpp"
#include <locale>
#include <sstream>

namespace
{
	// Counts the number of words separated by whitespace in the string.
	unsigned int countWords(const std::string& s)
	{
		unsigned int wordCount = 0;
		bool traversingWord = false;

		for (int i = 0; i < s.length(); ++i)
		{
			if (!std::isspace(s[i]))
			{
				traversingWord = false;
			}
			else if (std::isspace(s[i]) && !traversingWord)
			{
				wordCount += 1;
				traversingWord = true;
				// do something
			}
		}

		return wordCount;
	}

	void INVALID_INPUT()
	{
		std::cout << "INVALID" << std::endl;
	}
}

int main()
{
	// unsigned int testCountWords = countWords(" hello there are	five    words");
	// std::cout << testCountWords << std::endl;
	HashMap mainHashMap = HashMap();
	bool DEBUGGING_ON = false;
	std::string inputLine;
	unsigned int wordCount;
	std::string input1;
	std::string input2;
	std::string input3;

	while (true)
	{
		std::string input1 = "";
		std::string input2 = "";
		std::string input3 = "";
		std::getline(std::cin, inputLine);
		wordCount = countWords(inputLine);
		
		// If the input line has more than three words, then it is INVALID
		if (wordCount <= 3)
		{
			std::stringstream(inputLine) >> input1 >> input2 >> input3;
			// std::cout << input1 << std::endl << input2 << std::endl << input3 << std::endl;
			if (input1 == "CREATE" && input2 != "" && input3 != "")
			{
				if (mainHashMap.contains(input2))
				{
					std::cout << "EXISTS" << std::endl;
				}
				else
				{
					mainHashMap.add(input2, input3);
					std::cout << "CREATED" << std::endl;
				}

			} // End CREATE username password if
			else if (input1 == "LOGIN" && input2 != "" && input3 != "")
			{
				if (mainHashMap.contains(input2) && input3 == mainHashMap.value(input2))
				{
					std::cout << "SUCCEEDED" << std::endl;
				}	
				else
				{
					std::cout << "FAILED" << std::endl;
				}
			} // End LOGIN username password if
			else if (input1 == "REMOVE" && input2 != "" && input3 == "")
			{	
				if (mainHashMap.contains(input2))
				{
					mainHashMap.remove(input2);
					std::cout << "REMOVED" << std::endl;
				}
				else
				{
					std::cout << "NONEXISTENT" << std::endl;
				}
			} // End REMOVE username if
			else if (input1 == "CLEAR" && input2 == "" && input3 == "")
			{
				std::cout << "CLEARED" << std::endl;
				mainHashMap.deleteBuckets();
			} // End CLEAR if
			else if (input1 == "QUIT" && input2 == "" && input3 == "")
			{
				std::cout << "GOODBYE" << std::endl;
				break;
			} // End QUIT if
			else if (input1 == "DEBUG" && input2 != "" && input3 == "")
			{
				if (input2 == "ON")
				{
					if (DEBUGGING_ON)
					{
						std::cout << "ON ALREADY" << std::endl;
					}
					else
					{
						DEBUGGING_ON = true;
						std::cout << "ON NOW" << std::endl;
					}
				} // End ON if
				else if (input2 == "OFF")
				{
					if (DEBUGGING_ON)
					{
						DEBUGGING_ON = false;
						std::cout << "OFF NOW" << std::endl;
					}
					else
					{
						std::cout << "OFF ALREADY" << std::endl;
					}
				} // End OFF if
				else
				{
					INVALID_INPUT();
				}
			} // End DEBUG ON/OFF if
			else if (input1 == "LOGIN" && input2 == "COUNT" && input3 == "")
			{
				if (!DEBUGGING_ON)
				{
					INVALID_INPUT();
				}
				else
				{
					std::cout << mainHashMap.size() << std::endl;
				}
			} // End LOGIN COUNT if
			else if (input1 == "BUCKET" && input2 == "COUNT" && input3 == "")
			{
				if (!DEBUGGING_ON)
				{
					INVALID_INPUT();
				}
				else
				{
					std::cout << mainHashMap.bucketCount() << std::endl;
				}
			} // End BUCKET COUNT if
			else if (input1 == "LOAD" && input2 == "FACTOR" && input3 == "")
			{
				if (!DEBUGGING_ON)
				{
					INVALID_INPUT();
				}
				else
				{
					std::cout << mainHashMap.loadFactor() << std::endl;
				}
			} // End LOAD FACTOR if
			else if (input1 == "MAX" && input2 == "BUCKET" && input3 == "SIZE")
			{
				if (!DEBUGGING_ON)
				{
					INVALID_INPUT();
				}
				else
				{
					std::cout << mainHashMap.maxBucketSize() << std::endl;
				}
			} // End MAX BUCKET COUNT if
			else
			{
				INVALID_INPUT();
			}
		}
		else
		{
			INVALID_INPUT();
		}

	}

    return 0;
}

