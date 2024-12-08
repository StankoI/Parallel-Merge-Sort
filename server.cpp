#include <algorithm>
#include <iostream>
#include <future>
#include <sstream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <chrono>
//g++ -o server.exe server.cpp -lws2_32
#define PORT 2003

void insert(std::vector<int> &array, std::size_t index, std::size_t begin, std::size_t end)
{
    if (index > end)
    {
        throw std::invalid_argument("invalind index");
    }

    int element = array[index];
    int i = index;

    while (i > begin && array[i - 1] > element)
    {
        array[i] = array[i - 1];
        i--;
    }

    array[i] = element;
}

void insertionSort(std::vector<int> &array, std::size_t begin, std::size_t end)
{
    for (int i = begin; i < end; i++)
    {
        insert(array, i, begin, end);
    }
}

void insertionSortThreads(std::vector<int> &array, std::size_t numOfThreads)
{
    std::size_t size = array.size();
    numOfThreads = std::min(numOfThreads, size);
    std::size_t elementsPerThreads = size / numOfThreads;
    std::vector<std::thread> threads(numOfThreads);
    std::vector<std::size_t> indexes;

    for (std::size_t i = 0; i < numOfThreads; i++)
    {
        std::size_t startIndex = i * elementsPerThreads;
        std::size_t endIndex = (i + 1) * elementsPerThreads;
        if (i == numOfThreads - 1)
        {
            endIndex = size;
        }

        // std::cout << "s: " << startIndex << '\n'
        //           << "e: " << endIndex << '\n';

        indexes.push_back(startIndex);
        indexes.push_back(endIndex);
    }

    for (std::size_t i = 0; i < numOfThreads; i++)
    {
        threads[i] = std::thread(insertionSort, std::ref(array), indexes[i * 2], indexes[i * 2 + 1]);
    }
    // std::cout << "hello";

    for (size_t i = 0; i < numOfThreads; i++)
    {
        threads[i].join();
        // std::cout << "hello";
    }

    std::vector<int> sortedArray(size);
    std::vector<int> tempArray(size);

    for (size_t i = 0; i < numOfThreads; i++)
    {
        std::merge(tempArray.begin(), tempArray.begin() + indexes[2 * i],
                   array.begin() + indexes[2 * i], array.begin() + indexes[2 * i + 1],
                   sortedArray.begin());
        tempArray = sortedArray;
    }

    array = sortedArray;
}

void onClientConnect(SOCKET client)
{
    std::cout << "Client connected!\n";
    std::size_t numThreads, numElements;

    if (recv(client, (char *)&numThreads, sizeof(numThreads), 0) == 0) // ssize_t recv(int socket, void *buffer, size_t length, int flags);
    // recv() shall return the length of the message in bytes
    {
        std::cout << "Fail: recv from client number of threads!" << std::endl;
        ExitProcess(EXIT_FAILURE);
    }

    if (recv(client, (char *)&numElements, sizeof(numElements), 0) == 0)
    {
        std::cout << "Fail: recv from client number of elements!" << std::endl;
        ExitProcess(EXIT_FAILURE);
    }

    std::vector<int> arr(numElements);

    if (recv(client, (char *)arr.data(), arr.size() * sizeof(arr[0]), 0) == 0)
    {
        std::cout << "Fail: recv from client arr!" << std::endl;
        ExitProcess(EXIT_FAILURE);
    }

    insertionSortThreads(arr, numThreads);

    if (send(client, (const char *)arr.data(), arr.size() * sizeof(arr[0]), 0) == -1)
    {
        std::cout << "Fail: Send to client!" << std::endl;
        ExitProcess(EXIT_FAILURE);
    }

    closesocket(client);
    std::cout << "Client disconnected." << std::endl;
}

int main()
{
    WSADATA wsa;
    SOCKADDR_IN server_addr, client_addr;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cout << "WSAStartup()" << std::endl;
        ExitProcess(EXIT_FAILURE);
    }

    const SOCKET server = socket(AF_INET, SOCK_STREAM, 0); // TCP (SOCK_STREAM)
    if (server == -1)
    {
        std::cout << "Create socket" << std::endl;
        ExitProcess(EXIT_FAILURE);
    }

    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (bind(server, reinterpret_cast<SOCKADDR *>(&server_addr), sizeof(server_addr)) == -1)
    {
        shutdown(server, 2);
        std::cout << "Bind" << std::endl;
        ExitProcess(EXIT_FAILURE);
    }

    if ((listen(server, 0)) == -1)
    {
        std::cout << "Listen" << std::endl;
        ExitProcess(EXIT_FAILURE);
    }

    std::cout << "Listening for incoming connections..." << std::endl;

    int clientAddrSize = sizeof(client_addr);

    for (;;)
    {
        SOCKET client;

        if ((client = accept(server, reinterpret_cast<SOCKADDR *>(&client_addr), &clientAddrSize)) != INVALID_SOCKET)
        {
            onClientConnect(client);
        }

        const int last_error = WSAGetLastError();

        if (last_error > 0)
        {
            std::cout << "Error: " << last_error << std::endl;
        }
    }
    ExitProcess(EXIT_SUCCESS);

    return 0;
}