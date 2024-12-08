#include <iostream>
#include <sstream>
#include <vector>
#include <WinSock2.h>
#include <ws2tcpip.h>

#define PORT 2003

int main()
{
    WSADATA wsa_data;
    SOCKADDR_IN addr;

    if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0)
    {
        std::cout << "Failed: WSAStartup()\n";
        ExitProcess(EXIT_FAILURE);
    }

    const SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == -1)
    {
        std::cout << "Failed: Create socket\n";
        ExitProcess(EXIT_FAILURE);
    }

    // InetPton(AF_INET, L"127.0.0.1", &addr.sin_addr.s_addr);
    InetPton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    if ((connect(server, reinterpret_cast<SOCKADDR *>(&addr), sizeof(addr))) == -1)
    {
        std::cout << "Failed: Connect to server!\n";
        ExitProcess(EXIT_FAILURE);
    }

    size_t num_threads, num_elements;
    std::cout << "Enter number of threads: ";
    std::cin >> num_threads;
    std::cout << "Enter the number of array elements: ";
    std::cin >> num_elements;
    if (num_threads > num_elements || num_threads == 0)
    {
        std::cout << "Fail: Threads are more than elements!\n";
        closesocket(server);
        ExitProcess(EXIT_FAILURE);
    }
    else
    {
        int element;
        std::vector<int> arr;
        std::cout << "Enter " << num_elements << " numbers: ";
        for (size_t i = 0; i < num_elements; i++)
        {
            std::cin >> element;
            arr.push_back(element);
        }
        if ((send(server, (const char *)&num_threads, sizeof(num_threads), 0)) == -1)
        {
            std::cout << "Send to server number of threads!\n";
            ExitProcess(EXIT_FAILURE);
        }
        if ((send(server, (const char *)&num_elements, sizeof(num_elements), 0)) == -1)
        {
            std::cout << "Send to server number of elements!\n";
            ExitProcess(EXIT_FAILURE);
        }

        if ((send(server, (const char *)arr.data(), arr.size() * sizeof(arr[0]), 0)) == -1)
        {
            std::cout << "Send to server arr data!\n";
            ExitProcess(EXIT_FAILURE);
        }
        std::cout << "Messages sent!" << std::endl;
        if (recv(server, (char *)arr.data(), num_elements * sizeof(arr[0]), 0) == 0)
        {
            std::cout << "Fail: recv from server!";
            ExitProcess(EXIT_FAILURE);
        }
        std::cout << "Sent message from server.\nSorted array: \n";
        for (size_t i = 0; i < num_elements; i++)
        {
            std::cout << arr[i] << " ";
        }
        std::cout << std::endl;
    }
    closesocket(server);
    WSACleanup();
    std::cout << "Socket closed." << std::endl;
    ExitProcess(EXIT_SUCCESS);
}