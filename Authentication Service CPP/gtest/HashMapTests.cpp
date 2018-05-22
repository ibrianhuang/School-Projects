// HashMapTests.cpp
//
// ICS 45C Fall 2016
// Project #3: Maps and Legends
//
// Write unit tests for your HashMap class here.  I've provided one test already,
// though I've commented it out, because it won't compile until you've implemented
// three things: the HashMap constructor, the HashMap destructor, and HashMap's
// size() member function.

#include <gtest/gtest.h>
#include "HashMap.hpp"

namespace
{
	const char alphabetArray[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	void fillMap(HashMap& hm, int num)
	{
		for (unsigned int i = 0; i < num; ++i)
		{
			std::string key{alphabetArray[i], 'k'};
			std::string value{alphabetArray[i], 'v'};
			hm.add(key, value);
		}
	}

	// void addValue(HashMap& hm, int n)
	// {
	// 	std::string key{alphabetArray[n], 'k'};
	// 	std::string value{alphabetArray[n], 'v'};
	// 	hm.add(key, value);
		
	// }
}

TEST(TestHashMap, sizeOfNewlyCreatedHashMapIsZero)
{
   HashMap empty;
   ASSERT_EQ(0, empty.size());
}

TEST(TestHashMap, copyConstructorCopiesNodes)
{
	HashMap h;
	h.add("Key", "Value");

	HashMap h2 = h;
	ASSERT_TRUE(h2.contains("Key"));
}

TEST(TestHashMap, assignmentOperatorCopiesNodes)
{
	HashMap h1;
	h1.add("Key1", "Value1");

	HashMap h2;
	h2.add("Key2", "Value2");
	
	h2 = h1;
	ASSERT_FALSE(h2.contains("Key2"));
	ASSERT_TRUE(h2.contains("Key1"));
}

TEST(TestHashMap, addRemoveSizeUpdates)
{

	HashMap h;
	ASSERT_EQ(0, h.size());

	h.add("Key1", "Value1");
	h.add("Key2", "Value2");
	ASSERT_EQ(2, h.size());

	h.remove("Key1");
	ASSERT_EQ(1, h.size());
}

TEST(TestHashMap, containsAddedNodes)
{
	HashMap h;
	h.add("Key1", "Value1");
	h.add("Key2", "Value2");
	ASSERT_TRUE(h.contains("Key1"));
	ASSERT_TRUE(h.contains("Key2"));
	ASSERT_FALSE(h.contains("Key"));
}


TEST(TestHashMap, addRehashesAfterLoadMax)
{
	HashMap h;
	unsigned int x = h.bucketCount();
	// while (h.loadFactor() <= 0.8)
	// {
	// 	addValue(h, i);
	// 	++i;
	// 	std::cout << h.loadFactor() << std::endl;
	// }
	fillMap(h, 15); // Through multiple tests, it rehashes once with these 15 adds
	unsigned int y = h.bucketCount();
	ASSERT_GT(y, x);
}

TEST(TestHashMap, nodesRemovedFromHashMap)
{
	HashMap h;
	h.add("Key1", "Value1");
	h.add("Key2", "Value2");

	h.remove("Key1");
	ASSERT_TRUE(h.contains("Key2"));
	ASSERT_FALSE(h.contains("Key1"));
}

// Checking whether deleting an empty HashMap causes an error
TEST(TestHashMap, deletingEmptyBuckets)
{
	HashMap h;
	h.deleteBuckets();
}

TEST(TestHashMap, deletingBucketsRemovesNodes)
{
	HashMap h;
	h.add("Key1", "Value1");
	h.add("Key2", "Value2");

	h.deleteBuckets();
	ASSERT_FALSE(h.contains("Key2"));
	ASSERT_FALSE(h.contains("Key1"));
}

namespace
{
	unsigned int trivialHasher(const std::string& key)
	{
		if (key == "Key1")
		{
			return 8;
		}

		return 10;
	}
}

TEST(TestHashMap, testMaxBucketSize)
{
	HashMap h = HashMap(trivialHasher);
	h.add("Key", "Value");

	h.add("Key1", "Value1");
	h.add("Key2", "Value2");
	h.add("Key3", "Value1");

	ASSERT_EQ(3, h.maxBucketSize());
}