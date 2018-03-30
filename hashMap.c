/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Jesse McKenna
 * Date: 2018-3-8
 */

#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

int hashFunction1(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += key[i];
    }
    return r;
}

int hashFunction2(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += (i + 1) * key[i];
    }
    return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink* hashLinkNew(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link
 */
static void hashLinkDelete(HashLink* link)
{
    free(link->key);
    free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap* map, int capacity)
{
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink*) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        map->table[i] = NULL;
    }
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map
 */
void hashMapCleanUp(HashMap* map)
{
    // FIXED
	assert(map != NULL);

	HashLink* current;
	HashLink* delete;
	for (int i = 0, n = hashMapCapacity(map); i < n; i++) // for each bucket
	{
		current = map->table[i];
		while (current != NULL) // delete all links in bucket
		{
			delete = current;
			current = current->next;
			hashLinkDelete(delete);
		}	
	}
	free(map->table);
	map->size = 0;
	map->capacity = 0;
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap* hashMapNew(int capacity)
{
    HashMap* map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map
 */
void hashMapDelete(HashMap* map)
{
    hashMapCleanUp(map);
    free(map);
}

/**
 * Returns a pointer to the value of the link with the given key. Returns NULL
 * if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return Link value or NULL if no matching link.
 */
int* hashMapGet(HashMap* map, const char* key)
{
    // FIXED
	assert(map != NULL);

	int tableIndex = HASH_FUNCTION(key) % hashMapCapacity(map);
	if (tableIndex >= hashMapCapacity(map))
	{
		tableIndex -= hashMapCapacity(map); // in case % does not work as expected
	}
	HashLink* current = map->table[tableIndex];
	
	while (current != NULL)
	{
		if (strcmp(key, current->key) == 0) // target key found
		{
			return &(current->value); // return pointer to value
		}
		else
		{
			current = current->next;
		}
	}

	return NULL; // target key not found
}

/**
 * Resizes the hash table to have a number of buckets equal to the given
 * capacity. After allocating the new table, all of the links need to be
 * rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap* map, int capacity)
{
    // FIXED
	assert(map != NULL);

	// Replace table with new, larger table
	HashLink** oldTable = map->table; // preserve old table for copying values over
	int oldCapacity = map->capacity;
	
	map->table = malloc(sizeof(HashLink*) * capacity); // point map to new table
	map->capacity = capacity;
	map->size = 0; // hashMapPut() will re-increment the size again

	for (int i = 0; i < capacity; i++)
	{
		map->table[i] = NULL; // initialize all buckets with NULL
	}
	
	// Copy all existing values
	HashLink* current;
	HashLink* delete;
	for (int i = 0; i < oldCapacity; i++) // for each bucket in old table
	{
		current = oldTable[i];
		while (current != NULL) // copy over each link in the bucket's linked list
		{
			hashMapPut(map, current->key, current->value);
			delete = current;
			current = current->next;
			hashLinkDelete(delete); // free the link that was just copied
		}
	}

	free(oldTable);
}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @param value
 */
void hashMapPut(HashMap* map, const char* key, int value)
{
    // FIXED
	assert(map != NULL);

	int tableIndex = HASH_FUNCTION(key) % hashMapCapacity(map);
	if (tableIndex >= hashMapCapacity(map))
	{
		tableIndex -= hashMapCapacity(map); // in case % does not work as expected
	}
	
	HashLink* current = map->table[tableIndex];

	if (current == NULL) // header link is NULL; create new link here
	{
		map->table[tableIndex] = hashLinkNew(key, value, NULL);
	}
	else // iterate through linked list until existing key or end of list found
	{
		HashLink* previous;
		while (current != NULL)
		{
			if (strcmp(key, current->key) == 0) // existing key found
			{
				current->value = value; // update value and return
				return;
			}
			else
			{
				previous = current;
				current = current->next;
			}
		}

		// current is now at the last non-NULL link in the linked list
		previous->next = hashLinkNew(key, value, NULL);
	}

	map->size++;

	if (hashMapTableLoad(map) > MAX_TABLE_LOAD) // if map has exceeded max load
	{
		resizeTable(map, map->capacity * 2); // double its capacity
	}
}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map
 * @param key
 */
void hashMapRemove(HashMap* map, const char* key)
{
    // FIXED
	assert(map != NULL);

	int tableIndex = HASH_FUNCTION(key) % hashMapCapacity(map);
	if (tableIndex >= hashMapCapacity(map))
	{
		tableIndex -= hashMapCapacity(map); // in case % does not work as expected
	}
	
	HashLink* current = map->table[tableIndex];
	HashLink* previous = current;
	while (current != NULL)
	{
		if (strcmp(key, current->key) == 0) // target key found
		{
			if (previous == current) // removing first link
			{
				map->table[tableIndex] = current->next;
			}
			else // removing a subsequent link
			{
				previous->next = current->next; // remove current from list
			}
			
			hashLinkDelete(current); // remove target key and return
			current = NULL;
			map->size--;
			return;
		}
		else
		{
			previous = current;
			current = current->next;
		}
	}

	// Target key not found, do nothing
}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap* map, const char* key)
{
    // FIXED
	assert(map != NULL);

	int tableIndex = HASH_FUNCTION(key) % hashMapCapacity(map);
	if (tableIndex >= hashMapCapacity(map))
	{
		tableIndex -= hashMapCapacity(map); // in case % does not work as expected
	}
	HashLink* current = map->table[tableIndex];

	while (current != NULL)
	{
		if (strcmp(key, current->key) == 0) // target key found
		{
			return 1;
		}
		else
		{
			current = current->next;
		}
	}

	return 0; // target key not found
}

/**
 * Returns the number of links in the table.
 * @param map
 * @return Number of links in the table.
 */
int hashMapSize(HashMap* map)
{
    // FIXED
	assert(map != NULL);
    return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap* map)
{
    // FIXED
	assert(map != NULL);
    return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap* map)
{
    // FIXED
	assert(map != NULL);
	
	int empty = 0;
	for (int i = 0, n = hashMapCapacity(map); i < n; i++)
	{
		if (map->table[i] == NULL)
		{
			empty++;
		}
	}
	return empty;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map
 * @return Table load.
 */
float hashMapTableLoad(HashMap* map)
{
    // FIXED
	assert(map != NULL);
	
	float ratio = hashMapSize(map); // ratio = size / capacity
	ratio /= hashMapCapacity(map); // use two steps to prevent integer division
	
	return ratio;
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map
 */
void hashMapPrint(HashMap* map)
{
    for (int i = 0; i < map->capacity; i++)
    {
        HashLink* link = map->table[i];
        if (link != NULL)
        {
            printf("\nBucket %i ->", i);
            while (link != NULL)
            {
                printf(" (%s, %d) ->", link->key, link->value);
                link = link->next;
            }
        }
    }
    printf("\n");
}