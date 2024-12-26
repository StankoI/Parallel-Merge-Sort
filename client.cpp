#include <iostream>
#include <sstream>
#include <vector>
#include <WinSock2.h>
#include <ws2tcpip.h>
//g++ -o client.exe client.cpp -lws2_32

#define PORT 2003

class Client
{
public:
    Client(const std::string &server_ip, int port)
    {
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0)
        {
            std::cout << "Failed: WSAStartup()\n";
            ExitProcess(EXIT_FAILURE);
        }

        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1)
        {
            std::cout << "Failed: Create socket\n";
            ExitProcess(EXIT_FAILURE);
        }

        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        InetPton(AF_INET, server_ip.c_str(), &addr.sin_addr.s_addr);
    }

    ~Client()
    {
        closesocket(server_socket);
        WSACleanup();
    }

    bool connectToServer()
    {
        if ((connect(server_socket, reinterpret_cast<SOCKADDR *>(&addr), sizeof(addr))) == -1)
        {
            std::cout << "Failed: Connect to server!\n";
            return false;
        }
        return true;
    }

    bool sendData(const std::vector<int> &arr, size_t num_threads, size_t num_elements)
    {
        if (send(server_socket, (const char *)&num_threads, sizeof(num_threads), 0) == -1)
        {
            std::cout << "Failed: Send number of threads!\n";
            return false;
        }

        if (send(server_socket, (const char *)&num_elements, sizeof(num_elements), 0) == -1)
        {
            std::cout << "Failed: Send number of elements!\n";
            return false;
        }

        if (send(server_socket, (const char *)arr.data(), arr.size() * sizeof(arr[0]), 0) == -1)
        {
            std::cout << "Failed: Send array data!\n";
            return false;
        }

        std::cout << "Messages sent!" << std::endl;
        return true;
    }

    bool receiveData(std::vector<int> &arr, size_t num_elements)
    {
        if (recv(server_socket, (char *)arr.data(), num_elements * sizeof(arr[0]), 0) == 0)
        {
            std::cout << "Failed: recv from server!\n";
            return false;
        }

        std::cout << "Received message from server.\nSorted array: \n";
        return true;
    }

private:
    SOCKET server_socket;
    SOCKADDR_IN addr;
};

class Sorter
{
public:
    static void printArray(const std::vector<int> &arr)
    {
        for (size_t i = 0; i < arr.size(); ++i)
        {
            std::cout << arr[i] << " ";
        }
        std::cout << std::endl;
    }
};

int main()
{
    std::string server_ip = "127.0.0.1"; // local host
    Client client(server_ip, PORT);

    if (!client.connectToServer())
    {
        ExitProcess(EXIT_FAILURE);
    }

    size_t num_threads, num_elements;
    std::cout << "Enter number of threads: ";
    std::cin >> num_threads;
    std::cout << "Enter the number of array elements: ";
    std::cin >> num_elements;

    if (num_threads > num_elements || num_threads == 0)
    {
        std::cout << "Fail: Threads are more than elements or threads == 0!\n";
        ExitProcess(EXIT_FAILURE);
    }

    int element;
    std::vector<int> arr;
    std::cout << "Enter " << num_elements << " numbers: ";
    for (size_t i = 0; i < num_elements; ++i)
    {
        std::cin >> element;
        arr.push_back(element);
    }

    if (!client.sendData(arr, num_threads, num_elements))
    {
        ExitProcess(EXIT_FAILURE);
    }

    if (!client.receiveData(arr, num_elements))
    {
        ExitProcess(EXIT_FAILURE);
    }

    Sorter::printArray(arr);

    return 0;
}
