/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Jesse McKenna
 * Date: 2018-3-11
 */

#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    // FIXED
	char* word = nextWord(file);
	while (word != NULL) // for each word in file
	{
		for (int i = 0, n = strlen(word); i < n; i++)
		{
			word[i] = tolower(word[i]); // convert all characters to lower case
		}

		if (!hashMapContainsKey(map, word)) // ensure word is not already in map
		{
			hashMapPut(map, word, 0); // add word to map with initial value 0;
			// value will represent its Levenshtein distance from user input
			// (see: https://en.wikipedia.org/wiki/Levenshtein_distance)
		}

		// Be sure to free the word after you are done with it here.
		free(word);
		word = nextWord(file); // get next word
	}
}

int getMin(int a, int b, int c)
{
	if (a < b && a < c)
	{
		return a;
	}
	else if (b < c)
	{
		return b;
	}
	else
	{
		return c;
	}
}

/**
 * Prints the concordance of the given file and performance information. Uses
 * the file input1.txt by default or a file name specified as a command line
 * argument.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    // FIXED
    HashMap* map = hashMapNew(1000);
    
    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);
    
    char inputBuffer[256];
    int quit = 0;
    while (!quit)
    {
        printf("Enter a word or \"quit\" to quit: ");
        scanf("%s", inputBuffer);
        
		if (strcmp(inputBuffer, "quit") == 0)
		{
			quit = 1;
		}
		else
		{
			// Implement the spell checker code here..

			for (int i = 0, n = strlen(inputBuffer); i < n; i++)
			{
				inputBuffer[i] = tolower(inputBuffer[i]); // case-insensitive
			}

			// If word is in dictionary, print success message
			if (hashMapContainsKey(map, inputBuffer))
			{
				printf("Word spelled correctly.\n");
			}
			// If not, print closest matching words using Levenshtein distance
			// (reference: https://people.cs.pitt.edu/~kirk/cs1501/Pruhs/Spring2006/assignments/editdistance/Levenshtein%20Distance.htm)
			else
			{
				HashLink* current; // to iterate through dictionary
				
				int** matrix; // matrix used to compute Levenshtein distance
				int rowCount; // helper variables for working with matrix
				int colCount;

				// Create a simple struct to store the closest-matching words
				const int MATCHES = 5; // number of close matches to display
				HashLink** matches = malloc(MATCHES * sizeof(HashLink*));
				HashLink* weakestMatch = NULL; // furthest-distance match stored
				int matchesCount = 0; // words stored in matches array

				for (int i = 0, n = hashMapCapacity(map); i < n; i++)
				{
					current = map->table[i];
					while (current != NULL) // for each dictionary entry
					{
						// Create 2D matrix of size:
						// (dictionary entry length + 1) x (input length + 1)
						rowCount = strlen(current->key) + 1;
						colCount = strlen(inputBuffer) + 1;
						
						matrix = malloc(rowCount * sizeof(int*));
						for (int i = 0; i < rowCount; i++)
						{
							matrix[i] = malloc(colCount * sizeof(int));
						}

						// Set first row and column values to 0, 1, ..., n
						for (int i = 0; i < rowCount; i++)
						{
							matrix[i][0] = i;
						}
						for (int j = 0; j < colCount; j++)
						{
							matrix[0][j] = j;
						}

						// Calculate intermediate distances from previous values
						// (start at 1 because first row/column already filled)
						for (int i = 1; i < rowCount; i++)
						{
							for (int j = 1; j < colCount; j++)
							{
								// If no change needed, set to previous distance
								if (current->key[i - 1] == inputBuffer[j - 1])
								{
									matrix[i][j] = matrix[i - 1][j - 1];
								}
								// If change needed, set to minimum of three
								// adjacent previous distances, plus 1
								else
								{
									matrix[i][j] = getMin(matrix[i - 1][j - 1],
														  matrix[i - 1][j],
														  matrix[i][j - 1]) + 1;
								}
							}
						}

						// Value in last matrix cell is final distance
						current->value = matrix[rowCount - 1][colCount - 1];

						// Store as match if closer than existing matches
						if (matchesCount < MATCHES) // initial fill of matches
						{
							matches[matchesCount] = current;
							matchesCount++;

							if (weakestMatch == NULL || 
								current->value < weakestMatch->value)
							{
								weakestMatch = current;
							}
						}
						else if (current->value < weakestMatch->value)
						{
							// Current value is better than the weakest match
							HashLink* newWeakest = current;
							for (int i = 0; i < MATCHES; i++)
							{
								if (matches[i] == weakestMatch)
								{
									matches[i] = current; // replace weakest
								}
								else if (matches[i]->value > newWeakest->value)
								{
									newWeakest = matches[i]; // find new weakest
								}
							}
							weakestMatch = newWeakest;
						}
						
						// Free matrix in memory
						for (int i = 0; i < rowCount; i++)
						{
							free(matrix[i]);
						}
						free(matrix);

						current = current->next; // go to next dictionary entry
					}
				}

				// Print closest matches
				for (int i = 0; i < MATCHES; i++)
				{
					printf("Did you mean %s?\n", matches[i]->key);
				}

				free(matches);
			}
		}
    }
    
    hashMapDelete(map);
    return 0;
}