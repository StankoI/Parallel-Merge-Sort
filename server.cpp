#include <algorithm>
#include <iostream>
#include <future>
#include <sstream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <chrono>

#define port 2003

void insert(std::vector<int>& array, std::size_t index)
{
    int element = array[index];
    int i = index;

    while (i > 0 && array[i - 1] > element)
    {
        array[i] = array[i - 1];
        i--;
    }

    array[i] = element;
}


void insertSort(std::vector<int>& array, std::size_t size)
{
    for(int i = 0; i < size; i++)
    {
        insert(array,i);
    }
}

void insertSortTreads(std::vector<int>& array, std::size_t numOfThreads)
{
    std::size_t size = array.size();    
    std::size_t elementsPerThreads = size/numOfThreads;
    std::vector<std::thread> threads(numOfThreads);
    std::vector<std::size_t> indexes;

    
    

}

int main()
{
    std::vector<int> a{7,4,3,1};
    // insertsort(a,4);

    for(int el: a )
    {
        std::cout << el << " ";
    }

    return 0;
}