#include "HashMap.hpp"
#include <iostream>
#include <cstring>

namespace
{
	unsigned int default_hasher(const std::string& key)
	{
		unsigned int hash = 0;

		for(int i = 0; i < key.length(); ++i)
		{
			hash += (10^i)*((unsigned int)key[i]);
		}

		hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
		hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
		hash = ((hash >> 16) ^ hash);
		return hash;
	}


	
}

HashMap::HashMap()
	: hasher{default_hasher}, buckets{new Node*[initialBucketCount]}, mapSize{0}, mapBucketCount{initialBucketCount}
{
	for (int i = 0; i < mapBucketCount; ++i)
	{
		buckets[i] = nullptr;
	}
}

HashMap::HashMap(HashFunction hasher)
	: hasher{hasher}, buckets{new Node*[initialBucketCount]}, mapSize{0}, mapBucketCount{initialBucketCount}
{
	for (int i = 0; i < mapBucketCount; ++i)
	{
		buckets[i] = nullptr;
	}
}

// copy constructor may not copy nodes properly
HashMap::HashMap(const HashMap& hm)
: hasher{hm.hasher}, buckets{new Node*[hm.mapBucketCount]}, mapSize{hm.mapSize}, mapBucketCount{hm.mapBucketCount}
{
	copyBuckets(hm);

}	

// could use some work bb
HashMap::~HashMap()
{
	// Traverses the buckets and deletes the linked lists in each buckets
	deleteBuckets();

	delete[] buckets;
}

// assignment operator not done
HashMap& HashMap::operator=(const HashMap& hm)
{
	if (this != &hm)
	{
		hasher = hm.hasher;
		mapSize = hm.mapSize;
		mapBucketCount = hm.mapBucketCount;

		deleteBuckets();
		delete[] buckets;

		buckets = new Node*[hm.mapBucketCount];
		copyBuckets(hm);
	}

	return *this;
}

// could go NEED TO REHASH
void HashMap::add(const std::string& key, const std::string& value)
{
	if (!contains(key))
	{
		unsigned int index = bucketIndex(hasher(key), mapBucketCount);
		Node* currentNode = buckets[index];

		if (currentNode == nullptr)
		{
			currentNode = new Node{key, value, nullptr};
			buckets[index] = currentNode;
		}
		else
		{
			Node* nextNode = currentNode;
			currentNode = new Node{key, value, nextNode};
			buckets[index] = currentNode;
		}

		mapSize += 1;

		if (loadFactor() > 0.8)
		{
			rehashBuckets();
		}
	}

}

// could go bad
void HashMap::remove(const std::string& key)
{
	if (contains(key))
	{
		unsigned int index = bucketIndex(hasher(key), mapBucketCount);
		Node* currentNode = buckets[index];
		Node* nextNode;

		// If the first node in the linked list is the node to be removed
		if (currentNode->key == key)
		{
			if (currentNode->next == nullptr)
			{
				delete currentNode;
				buckets[index] = nullptr;
			}
			else
			{
				nextNode = currentNode->next;
				delete currentNode;
				buckets[index] = nextNode;
			}
		}
		else
		{
			nextNode = currentNode->next;
			Node* previousNode = currentNode;
			currentNode = nextNode;

			// If the key is in the linked list, this should not reach the end condition
			while (currentNode->next != nullptr)
			{
				nextNode = currentNode->next;

				if (currentNode->key == key)
				{
					previousNode->next = nextNode;
					// potential memory leak
					delete currentNode;
					break;
				}

				previousNode = currentNode;
				currentNode = nextNode;
			}
		}

		mapSize -= 1;
	}
}

// may not work
bool HashMap::contains(const std::string& key) const
{
	unsigned int index = bucketIndex(hasher(key), mapBucketCount);
	Node* currentNode = buckets[index];

	// If the linked list in the bucket is empty
	if (currentNode == nullptr)
	{
		return false;
	}

	// If the first element in the linked list is the key
	if (currentNode->key == key)
	{
		return true;
	}

	Node* nextNode;

	while (currentNode->next != nullptr)
	{
		nextNode = currentNode->next;

		if (currentNode->key == key)
		{
			return true;
		}

		currentNode = nextNode;
	}

	return false;
}

// could break
std::string HashMap::value(const std::string& key) const
{
	std::string output = "";
	if (!contains(key))
	{
		return output;
	}
	else
	{
		unsigned int index = bucketIndex(hasher(key), mapBucketCount);
		Node* currentNode = buckets[index];

		if (currentNode->key == key)
		{
			return currentNode->value;
		}
		else
		{
			Node* nextNode;

			while (currentNode->next != nullptr)
			{
				nextNode = currentNode->next;

				if (currentNode->key == key)
				{
					return currentNode->value;
				}

				currentNode = nextNode;
			}
		}
	}

	return output;

}

// could go bad
unsigned int HashMap::size() const
{
	return mapSize;
}

// coul go bad
unsigned int HashMap::bucketCount() const
{
	return mapBucketCount;
}

// could go bad
double HashMap::loadFactor() const
{
	return mapSize/(double)mapBucketCount;
}

// yee
unsigned int HashMap::maxBucketSize() const
{
	unsigned int maxSize = 0;
	unsigned int count;

	for (int i = 0; i < mapBucketCount; ++i)
	{
		Node* currentNode = buckets[i];

		if (currentNode != nullptr)
		{
			count = 1;
			currentNode = currentNode->next;

			while (currentNode != nullptr)
			{
				count += 1;
				currentNode = currentNode->next;
			}

			if (count > maxSize)
			{
				maxSize = count;
			}
		}
	}

	return maxSize;
}

// Helper function for copy/assignment constructors. Copies the HashTable's bucket's/nodes
void HashMap::copyBuckets(const HashMap& hm)
{
	for (int i = 0; i < mapBucketCount; ++i)
	{
		Node* currentNode = hm.buckets[i];
		Node* nextNode;

		if (hm.buckets[i] != nullptr)
		{
			this->buckets[i] = new Node;

			Node* copyNode = this->buckets[i];

			while (currentNode != nullptr)
			{
				nextNode = currentNode->next;

				copyNode->key = currentNode->key;
				copyNode->value = currentNode->value;

				if (nextNode != nullptr)
				{
					copyNode->next = new Node;
				}
				else
				{
					copyNode->next = nullptr;
				}

				currentNode = nextNode;
			}
		}
		else
		{
			this->buckets[i] = nullptr;
		}
	}
}

void HashMap::deleteBuckets()
{	
	for(int i = 0; i < mapBucketCount; ++i)
	{
		Node* currentNode = buckets[i];
		Node* deleteNode;

		while (currentNode != nullptr)
		{
			deleteNode = currentNode;
			currentNode = currentNode->next;
			delete deleteNode;
		}

		buckets[i] = nullptr;
	}
}

unsigned int HashMap::bucketIndex(const unsigned int& hash, const unsigned int&bucketCount) const
{
	return hash%bucketCount;
}

void HashMap::rehashBuckets()
{
	unsigned int newBucketCount = (mapBucketCount*2) + 1;
	Node** newBuckets = new Node*[newBucketCount];
	for (int i = 0; i < newBucketCount; ++i)
	{
		newBuckets[i] = nullptr;
	}

	for (int i = 0; i < mapBucketCount; ++i)
	{
		Node* sourceNode = buckets[i];

		while (sourceNode != nullptr)
		{
			Node* destinationNode;

			unsigned int newHash = hasher(sourceNode->key);
			unsigned int newBucketIndex = bucketIndex(newHash, newBucketCount);

			if (newBuckets[newBucketIndex] == nullptr)
			{
				destinationNode = new Node{sourceNode->key, sourceNode->value, nullptr};
				newBuckets[newBucketIndex] = destinationNode;
			}
			else
			{
				Node* nextNode = newBuckets[newBucketIndex];
				destinationNode = new Node{sourceNode->key, sourceNode->value, nextNode};
				newBuckets[newBucketIndex] = destinationNode;
			}

			sourceNode = sourceNode->next;
			
		}
	}

	deleteBuckets();
	delete[] buckets;

	mapBucketCount = newBucketCount;
	buckets = newBuckets;
}