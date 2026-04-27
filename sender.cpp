#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

std::string buildaddr(std::string device, int port) {
    return device + ":" + std::to_string(port);
}

int port = 0;
std::string device = "";

int main() {

    std::ifstream configFile("config.txt");
    if (!configFile) {
        std::cerr << "config file failed to load or dosnt exist" << std::endl;
        return 1;
    }
    else if (configFile.is_open()) {
        std::string line;
        while (std::getline(configFile, line)) {
            std::string key = line.substr(0, line.find('='));
            std::string value = line.substr(line.find('=') + 1);
            if (key == "PORT") {
                port = std::stoi(value);
                std::cout << "PORT: " << port << std::endl;
            }
            }
        }

    
    std::string device = "130.230.155.198";
    std::string address = buildaddr(device, port);

    int soc = socket (AF_INET, SOCK_DGRAM, 0);
    if (soc == -1) {
        std::cerr << "socket failed" << std::endl;
        return 1;
    }


    sockaddr_in destination;
    destination.sin_family = AF_INET;
    destination.sin_port = htons(port);
    inet_pton(AF_INET, device.c_str(), &destination.sin_addr);

    sendto(soc, address.c_str(), address.size(), 0, (sockaddr*)&destination, sizeof(destination));
    std::cout << "sent to: " << device << " from " << port << std::endl;
    
    
    return 0;
}

 