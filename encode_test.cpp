#include <iostream>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h> 
}
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
        std::cerr << "could not open codec" << std::endl;
        return 1;
    }
    std::cout << "codec opened succesfully" << std::endl;

    return 0;
}