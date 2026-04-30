#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <fstream>

struct ChunkHeader {
    uint16_t packet_id;
    uint16_t chunk_index;
    uint16_t total_chunks;
    uint16_t data_size;
};

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

    uint8_t buffer[1408];
    sockaddr_in sender;
    socklen_t senderSize = sizeof(sender);

    uint8_t reassembly[1400 * 10];
    int chunks_received = 0;

    while (true){
        int bytes = recvfrom(soc, buffer, sizeof(buffer), 0, (sockaddr*)&sender, &senderSize);

        ChunkHeader header;
        memcpy(&header, buffer, sizeof(ChunkHeader));

        memcpy(reassembly + (header.chunk_index * 1400),
            buffer + sizeof(ChunkHeader),
            header.data_size);
        
        chunks_received ++;

        std::cout << "packet_id: " << header.packet_id 
        << " chunk: " << header.chunk_index 
        << "/" << header.total_chunks 
        << " size: " << header.data_size 
        << std::endl;
        
        if(chunks_received == header.total_chunks) {
            std::cout << "packet completed" << std::endl;
            chunks_received = 0;
        }
    
    }
    return 0;
}