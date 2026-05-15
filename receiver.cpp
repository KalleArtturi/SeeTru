#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <fstream>
#include<csignal>
#include <unistd.h>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h> 
}

struct ChunkHeader {
    uint16_t packet_id;
    uint16_t chunk_index;
    uint16_t total_chunks;
    uint16_t data_size;
};


int port = 0;


bool running = true;

void handle_signal(int sig) {
    running = false;
}

int main() {

    signal(SIGINT,handle_signal);

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

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));


    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "unable to find decoder" << std::endl;
        return 1;
    }

    AVCodecContext* c = avcodec_alloc_context3(codec);
    if(!c) {
        std::cerr << "failed to allocate context" << std::endl;
        return 1;
    }
    
    int open = avcodec_open2(c, codec , NULL);
    if(open < 0 ) {
        std::cerr << "unable to open decoding codec " << std::endl;
        return 1;
    }

    uint8_t buffer[1408];
    sockaddr_in sender;
    socklen_t senderSize = sizeof(sender);

    uint8_t reassembly[1400 * 60];

    int chunks_received = 0;
    int total_size = 0;

    while (running){
        int bytes = recvfrom(soc, buffer, sizeof(buffer), 0, (sockaddr*)&sender, &senderSize);
        if(bytes < 0) continue;
        if(bytes < (int)sizeof(ChunkHeader)) continue;
        
        ChunkHeader header;
        memcpy(&header, buffer, sizeof(ChunkHeader));

        if(header.total_chunks == 0 || header.total_chunks > 60) {
            std::cerr << "invalid header, dropping" << std::endl;
            continue;
        }
        if(header.chunk_index >= header.total_chunks) {
            std::cerr << "invalid chunk index, dropping" << std::endl;
            continue;
        }
        if(header.data_size == 0 || header.data_size > 1400) {
            std::cerr << "invalid data size, dropping" << std::endl;
            continue;
        }

        

        if(header.chunk_index * 1400 + header.data_size > sizeof(reassembly)) {
            std::cerr << "chunk too large, dropping" << std::endl;
            continue;
        }

        memcpy(reassembly + (header.chunk_index * 1400),
            buffer + sizeof(ChunkHeader),
            header.data_size);

        total_size += header.data_size;
        chunks_received ++;

        std::cout << "packet_id: " << header.packet_id 
        << " chunk: " << header.chunk_index 
        << "/" << header.total_chunks 
        << " size: " << header.data_size 
        << std::endl;
        
        if(chunks_received == header.total_chunks) {
            AVPacket* pkt = av_packet_alloc();
            av_new_packet(pkt, total_size);
            memcpy(pkt->data, reassembly, total_size);

            int ret = avcodec_send_packet(c, pkt);
            if(ret < 0) {
                char errbuf[64];
                av_strerror(ret, errbuf, sizeof(errbuf));
                std::cerr << "failed to send packet: " << errbuf << std::endl;
            }

            AVFrame* frame = av_frame_alloc();
            int ret2 = avcodec_receive_frame(c, frame);
            if(ret2 == 0) {
                std::cout << "decoded frame: " << frame->width << "x" << frame->height << std::endl;
            } 
            else if(ret2 == AVERROR(EAGAIN)) {
            } 
            else {
                std::cerr << "decode error" << std::endl;
            }
            av_frame_free(&frame);

            std::cout << "packet completed" << std::endl;
            chunks_received = 0;
            total_size = 0;
        }
    
    }

    close(soc);
    std::cout << "cleaned up, exiting" << std::endl;


    return 0;
}