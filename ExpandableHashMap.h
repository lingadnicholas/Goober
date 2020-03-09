#ifndef EXPANDABLEHASHMAP_H
#define EXPANDABLEHASHMAP_H
#include <list> 
#include <iostream>
#include <vector>
// ExpandableHashMap.h


// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5); // constructor
	~ExpandableHashMap(); // destructor; deletes all of the items in the hashma
	void reset(); // resets the hashmap back to 8 buckets, deletes all items
	int size() const; // return the number of associations in the hashmap

	// The associate method associates one item (key) with another (value).
	// If no association currently exists with that key, this method inserts
	// a new association into the hashmap with that key/value pair. If there is
	// already an association with that key in the hashmap, then the item
	// associated with that key is replaced by the second parameter (value).
	// Thus, the hashmap must contain no duplicate keys.
	void associate(const KeyType& key, const ValueType& value);


	// If no association exists with the given key, return nullptr; otherwise,
	// return a pointer to the value associated with that key. This pointer can be
	// used to examine that value, and if the hashmap is allowed to be modified, to
	// modify that value directly within the map (the second overload enables
	// this). Using a little C++ magic, we have implemented it in terms of the
	// first overload, which you must implement.

	// for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	// for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	// C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
	struct Pair {
		Pair(KeyType k, ValueType v)
			:m_key(k), m_value(v)
		{

		}
		KeyType m_key;
		ValueType m_value;
	};
	double m_maximumLoad; 
	int m_numBuckets; 
	int m_numAssociations; 
	int m_bucketsFilled; 
	std::vector<std::list<Pair>> m_associationsTable; //a vector of lists.

	//Empty all linked lists 
	void freeLists()
	{
		for (int n = 0; n < m_numBuckets; n++)
		{
			if (!(m_associationsTable[n]).empty())
			{
				auto it = m_associationsTable[n].begin();
				while (it != m_associationsTable[n].end())
				{
					it = m_associationsTable[n].erase(it);
				}
			}
		}
	}
	unsigned int retHash(const KeyType& k) const
	{
	unsigned int hasher(const KeyType& k); //prototype
	return hasher(k) % m_numBuckets; //Returns the bucketNum key should be in!
	}

};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
	: m_numBuckets(8), m_maximumLoad(maximumLoadFactor), m_numAssociations(0)
{
	m_associationsTable.resize(m_numBuckets); 
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	freeLists(); 
	int m_numBuckets = 8; 
	m_associationsTable.resize(m_numBuckets); //Create a new table with 8 buckets
	m_numAssociations = 0; 
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return m_numAssociations; //Return number of associations
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
	ValueType* pt1 = nullptr; //delete
	//If there is an association with the key already, update that value and return.
	ValueType* ptr = find(key);
	if (ptr != nullptr)
	{
		*ptr = value; 
		return; 
	}

	unsigned int bucketNum = retHash(key); //Returns a number between 0 and m_numBuckets

	//Check if creating a new association would overload the max load
	if (m_associationsTable[bucketNum].empty()) 
	{
		//Calculate new load
		m_bucketsFilled++; 
		double newLoad = (m_bucketsFilled) / (m_numBuckets); 
		if (newLoad > m_maximumLoad)
		{

			//Create a new table: 
			int newNumBuckets = m_numBuckets * 2; 
			int associations = m_numAssociations; 
			std::vector<std::list<Pair>> newTable;
			newTable.resize(newNumBuckets); 

			//Iterate through each of the linked lists, rehashing all the keys and storing into the new table. 
			for (int n = 0; n < m_numBuckets; n++)
			{
				if (!m_associationsTable[n].empty())
				{
					auto it = m_associationsTable[n].begin(); 
					while (it != m_associationsTable[n].end())
					{
						unsigned int newHash = retHash(it->m_key); 

						newTable[newHash].push_back(Pair(it->m_key, it->m_value)); 
						++it; 
					}
				}
			}

			//Free memory associated with the original, smaller hash map 
			reset();
			

			//Replace the current hash map with the new hash map
			m_numBuckets = newNumBuckets;
			m_numAssociations = associations;
			m_associationsTable.resize(m_numBuckets); 
			m_associationsTable = newTable; 
			//Rehash the most recent association according to the new bucket size. 
			bucketNum = retHash(key); 
		}
	}
	//Now insert.

	m_associationsTable[bucketNum].push_back(Pair(key, value));
	m_numAssociations++; 
	
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	if (m_numAssociations == 0) //If there is nothing, return a nullptr
		return nullptr; 

	unsigned int bucketNum = retHash(key); //Returns the bucketNum key should be in!
	if (!m_associationsTable[bucketNum].empty())
	{
		auto it = m_associationsTable[bucketNum].begin();
		while (it != m_associationsTable[bucketNum].end())
		{
			if (it->m_key == key)
			{
				return &(it->m_value); //Set ptr to point to the value of the key.
				break;
			}
			++it;
		}
	}
	return nullptr;  // Not found
}
#endif //EXPANDABLEHASHMAP_H

