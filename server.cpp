#include <algorithm>
#include <iostream>
#include <future>
#include <sstream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <chrono>
#include <stdexcept>

//g++ -o server.exe server.cpp -lws2_32

#define PORT 2003

class Sorter
{
private:
    static void devideArray(std::size_t numOfThreads, std::size_t elementsPerThreads, std::vector<std::size_t>& indexes , std::size_t size)
    {
        for (std::size_t i = 0; i < numOfThreads; i++)
        {
            std::size_t startIndex = i * elementsPerThreads;
            std::size_t endIndex = (i + 1) * elementsPerThreads;
            if (i == numOfThreads - 1)
            {
                endIndex = size;
            }

            indexes.push_back(startIndex);
            indexes.push_back(endIndex);
        }
    }

    static void getThreads(std::vector<std::thread>& threads, std::size_t numOfThreads, std::vector<int> &array, std::vector<std::size_t>& indexes)
    {
        for (std::size_t i = 0; i < numOfThreads; i++)
        {
            threads[i] = std::thread(insertionSort, std::ref(array), indexes[i * 2], indexes[i * 2 + 1]);
        }
    }

    static std::vector<int> mergeArray( std::vector<int> &array, std::size_t numOfThreads, std::vector<std::size_t>& indexes , std::size_t size)
    {
        
        std::vector<int> sortedArray(size);
        std::vector<int> tempArray(size);

        for (size_t i = 0; i < numOfThreads; i++)
        {
            std::merge(tempArray.begin(), tempArray.begin() + indexes[2 * i],
                       array.begin() + indexes[2 * i], array.begin() + indexes[2 * i + 1],
                       sortedArray.begin());
            tempArray = sortedArray;
        }

        return sortedArray;
    }

    static void joinThreads(std::vector<std::thread>& threads, std::size_t numOfThreads)
    {
        for (size_t i = 0; i < numOfThreads; i++)
        {
            threads[i].join();
        }
    }

public:
    static void insertionSort(std::vector<int> &array, std::size_t begin, std::size_t end)
    {
        for (int i = begin; i < end; i++)
        {
            insert(array, i, begin, end);
        }
    }

    static void insertionSortThreads(std::vector<int> &array, std::size_t numOfThreads)
    {
        std::size_t size = array.size();
        numOfThreads = std::min(numOfThreads, size);
        std::size_t elementsPerThreads = size / numOfThreads;
        std::vector<std::thread> threads(numOfThreads);
        std::vector<std::size_t> indexes;

        devideArray(numOfThreads, elementsPerThreads, indexes, size);
        getThreads(threads, numOfThreads, array, indexes);
        joinThreads(threads,numOfThreads);
        array = mergeArray(array,numOfThreads,indexes,size);
    }

private:
    static void insert(std::vector<int> &array, std::size_t index, std::size_t begin, std::size_t end)
    {
        if (index > end)
        {
            throw std::invalid_argument("invalid index");
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
};

class Server
{
public:
    Server()
    {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            std::cerr << "WSAStartup() failed." << std::endl;
            exit(EXIT_FAILURE);
        }

        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1)
        {
            std::cerr << "Socket creation failed." << std::endl;
            exit(EXIT_FAILURE);
        }

        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);

        if (bind(server_socket, reinterpret_cast<SOCKADDR *>(&server_addr), sizeof(server_addr)) == -1)
        {
            shutdown(server_socket, 2);
            std::cerr << "Binding failed." << std::endl;
            exit(EXIT_FAILURE);
        }

        if (listen(server_socket, 0) == -1)
        {
            std::cerr << "Listen failed." << std::endl;
            exit(EXIT_FAILURE);
        }

        std::cout << "Server is listening for incoming connections..." << std::endl;
    }

    ~Server()
    {
        closesocket(server_socket);
        WSACleanup();
    }

    void run()
    {
        int clientAddrSize = sizeof(client_addr);
        for (;;)
        {
            SOCKET client_socket = accept(server_socket, reinterpret_cast<SOCKADDR *>(&client_addr), &clientAddrSize);
            if (client_socket != INVALID_SOCKET)
            {
                handleClientConnection(client_socket);
            }

            const int last_error = WSAGetLastError();
            if (last_error > 0)
            {
                std::cerr << "Error: " << last_error << std::endl;
            }
        }
    }

private:
    void handleClientConnection(SOCKET client)
    {
        std::cout << "Client connected!" << std::endl;

        std::size_t numThreads, numElements;

        if (recv(client, (char *)&numThreads, sizeof(numThreads), 0) == 0)
        {
            std::cerr << "Failed to receive number of threads from client." << std::endl;
            closesocket(client);
            return;
        }

        if (recv(client, (char *)&numElements, sizeof(numElements), 0) == 0)
        {
            std::cerr << "Failed to receive number of elements from client." << std::endl;
            closesocket(client);
            return;
        }

        std::vector<int> arr(numElements);
        if (recv(client, (char *)arr.data(), arr.size() * sizeof(arr[0]), 0) == 0)
        {
            std::cerr << "Failed to receive array from client." << std::endl;
            closesocket(client);
            return;
        }

        Sorter::insertionSortThreads(arr, numThreads);

        if (send(client, (const char *)arr.data(), arr.size() * sizeof(arr[0]), 0) == -1)
        {
            std::cerr << "Failed to send data to client." << std::endl;
        }

        closesocket(client);
        std::cout << "Client disconnected." << std::endl;
    }

    SOCKET server_socket;
    SOCKADDR_IN server_addr, client_addr;
};

int main()
{
    Server server;
    server.run();

    // const size_t arraySize = 100000; // Размер на масива
    // std::vector<int> array(arraySize);
    // std::generate(array.begin(), array.end(), std::rand);

    // std::vector<int> arrayCopy = array;

    // auto startSingle = std::chrono::high_resolution_clock::now();
    // Sorter::insertionSort(array, 0, array.size());
    // auto endSingle = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> singleThreadDuration = endSingle - startSingle;

    // std::cout << "Single-threaded sort time: " << singleThreadDuration.count() << " seconds\n";

    // auto startMulti = std::chrono::high_resolution_clock::now();
    // Sorter::insertionSortThreads(arrayCopy, 32); 
    // auto endMulti = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> multiThreadDuration = endMulti - startMulti;

    // std::cout << "Multi-threaded sort time (32 threads): " << multiThreadDuration.count() << " seconds\n";
    
    return 0;
}
