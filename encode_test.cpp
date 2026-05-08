#include <iostream>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <opencv2/opencv.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h> 
}

struct ChunkHeader
{
    uint16_t packet_id;
    uint16_t chunk_index;
    uint16_t total_chunk;
    uint16_t data_size;
};

int main(){
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec){
        std::cerr << "codec not found" << std::endl;
        return 1;
    }
    std::cout << "found encoder: " << codec->name << std::endl;

    AVCodecContext* c = avcodec_alloc_context3(codec);
    if (!c) {
        std::cerr << "Codec allocation failed " << std::endl;
        return 1;
    }

    /*edit here to change parameters*/
    c->bit_rate = 2000000;
    c->time_base = (AVRational){1, 30};
    c->framerate = (AVRational){30, 1};
    c->width = 1280;
    c->height = 720;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    av_opt_set(c->priv_data, "preset", "ultrafast", 0);
    av_opt_set(c->priv_data, "tune", "zerolatency", 0);

    int open = avcodec_open2(c , codec, NULL);
    if(open < 0){
        std::cerr << "could not open encryption codec" << std::endl;
        return 1;
    }
    std::cout << "codec opened succesfully" << std::endl;

    AVFrame* frame = av_frame_alloc();
    if(!frame) {
        std::cerr << "Failure to allocate frame" << std::endl;
    }

    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;

    int buffer = av_frame_get_buffer(frame, 0);
    if(buffer < 0) {
    std::cerr << "Failure to allocate buffer" << std::endl;
    return 1;
    }
    std::cout << "buffer allocate succefully" << std::endl;


    int port = 0;
    std::string device = "";

    std::ifstream configFile("config.txt");
    if(!configFile) {
        std::cerr << "config file failed to load" << std::endl;
        return 1;
    }


    
    std::string line;
    while(std::getline(configFile, line)) {
        std::string key = line.substr(0, line.find('='));
        std::string value = line.substr(line.find('=') + 1);
        if(key == "PORT") {
            port = std::stoi(value);
        }
    }


    int soc = socket(AF_INET, SOCK_DGRAM, 0);
    if(soc == -1) {
        std::cerr << "socket failed" << std::endl;
        return 1;
    }
    sockaddr_in destination;
    destination.sin_family = AF_INET;
    destination.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &destination.sin_addr);
    

    AVPacket* packet = av_packet_alloc();
    if(!packet){
        std::cerr << "packet failed allocate" << std::endl;
        return 1;
    }
    std::cout << "packet allocated succefully" << std::endl;



    cv::VideoCapture cap(0);  // 0 = default camera
    
    if(!cap.isOpened()) {
        std::cerr << "camera failed to open" << std::endl;
        return 1;
    }



    for(int i = 0; i < 30; i++) {
        cv::Mat frame_bgr;
        cap >> frame_bgr;
        cv::Mat resized;
        cv::resize(frame_bgr, resized, cv::Size(1280, 720));
        cv::Mat yuv;
        cv::cvtColor(resized, yuv, cv::COLOR_BGR2YUV_I420);

        memcpy(frame->data[0], yuv.data, 1280 * 720);
        memcpy(frame->data[1], yuv.data + 1280 * 720, 1280 * 720 / 4);
        memcpy(frame->data[2], yuv.data + 1280 * 720 * 5/4, 1280 * 720 / 4);

        frame->pts = i;

        int send = avcodec_send_frame(c, frame);
        if(send < 0) {
            std::cerr << "failed to send frame" << std::endl;
            return 1;
        }


        int receive = avcodec_receive_packet(c, packet);
        if(receive < 0) {
            std::cerr << "failed to receive packet" << std::endl;
            return 1;
        }
        std::cout << "encoded packet, size: " << packet->size << std::endl;


        int total_chunks = (packet->size + 1399) / 1400;
        for(int j = 0; j < total_chunks; j++) {
            int offset = j * 1400;
            int chunk_size = std::min(1400, packet->size - offset);

            ChunkHeader header;
            header.packet_id = i;
            header.chunk_index = j;
            header.total_chunk = total_chunks;
            header.data_size = chunk_size;

            uint8_t udp_buffer[1408]; // header = 8 and 1400 bytes data
            memcpy(udp_buffer, &header, sizeof(ChunkHeader));
            memcpy(udp_buffer + sizeof(ChunkHeader), packet->data + offset, chunk_size);

            sendto(soc, udp_buffer, sizeof(ChunkHeader) + chunk_size, 0, (sockaddr*)&destination, sizeof(destination));
        }
        


    }



    return 0;
}