#include <stdio.h>
#include <string.h>

// 1. Defining Bit Array
#define BLOOM_SIZE 20
int bloom_filter[BLOOM_SIZE]; // all initialized to 0

void printBloom()
{
    for (int iterator = 0; iterator < BLOOM_SIZE; iterator++)
    {
        printf("%d ", bloom_filter[iterator]);
    }
    printf("\n");
}

// 2. Hash Functions
int hashFunctionOne(char *word)
{
    // Sum of ASCII values of characters in the word
    int sum = 0;
    for (int iterator = 0; word[iterator] != '\0'; iterator++)
    {
        sum += word[iterator]; // example: cat => 312
    }
    return sum % BLOOM_SIZE;
}

int hashFunctionTwo(char *word)
{
    // Weighted sum of ASCII values of characters in the word
    int sum = 0;
    for (int iterator = 0; word[iterator] != '\0'; iterator++)
    {
        sum += (iterator + 1) * word[iterator]; // example: cat => 641
    }
    return sum % BLOOM_SIZE;
}

// 3. Insertion
void insert(char *word)
{
    int firstIndex = hashFunctionOne(word);
    int secondIndex = hashFunctionTwo(word);

    bloom_filter[firstIndex] = 1;
    bloom_filter[secondIndex] = 1;

    printf("Inserted %s at indices %d and %d\n", word, firstIndex, secondIndex);
    printBloom();
}

// 4. Searching
int search(char *word)
{
    int firstIndex = hashFunctionOne(word);
    int secondIndex = hashFunctionTwo(word);

    if (bloom_filter[firstIndex] == 1 && bloom_filter[secondIndex] == 1)
    {
        printf("\"%s\" is possibly present at positions %d %d\n",
               word, firstIndex, secondIndex);
        return 1; // Possibly present
    }
    else
    {
        printf("\"%s\" is definitely not present at positions %d %d\n",
               word, firstIndex, secondIndex);
        return 0; // Definitely not present
    }
}

int main()
{
    memset(bloom_filter, 0, sizeof(bloom_filter));

    insert("cat");
    insert("dog");
    insert("rat");
    insert("bat");

    search("cat");

    // force a false positive
    bloom_filter[9] = 1;
    bloom_filter[18] = 1;
    search("cow"); 

    return 0;
}