#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <fstream>



int port = 0;

int main() {

    int soc = socket (AF_INET, SOCK_DGRAM, 0);
    if (soc == -1) {
        std::cerr << "socket failed " << std::endl;
        return 1;
    }
    
    std::ifstream configFile("config.txt");
    if (!configFile) {
        std::cerr << "config file failed to load" << std::endl;
    }
    if (configFile.is_open()) {
        std::string line;
        while (std::getline(configFile, line)) {
            std::string key = line.substr(0, line.find('='));
            std::string value = line.substr(line.find('=') + 1);
            if (key == "PORT"){
                port = std::stoi(value);
                std::cout << "PORT: " << port << std::endl;
            }
            else {
                std::cerr << "unknown key" << key << std::endl;
            }
        }

    }




    sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    
    client_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(soc, (sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
        std::cerr << "bind failed " << std::endl;
        return 1;
    }

    char buffer[1408];
    sockaddr_in sender;
    socklen_t senderSize = sizeof(sender);

    while (true){
        int bytes = recvfrom(soc, buffer, sizeof(buffer), 0, (sockaddr*)&sender, &senderSize);
        std::cout << "received " << bytes << " bytes from " << inet_ntoa(sender.sin_addr) << std::endl;
    }
    return 0;
}