#include <opencv2/opencv.hpp>

int main() {
    cv::VideoCapture cap(0);  // 0 = default camera
    
    if(!cap.isOpened()) {
        std::cerr << "camera failed to open" << std::endl;
        return 1;
    }
    
    std::cout << "camera opened!" << std::endl;
    cv::Mat frame;
    cap >> frame;

    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(1280, 720));
    
    cv::Mat yuv;
    cv::cvtColor(resized, yuv, cv::COLOR_BGR2YUV_I420);

    std::cout << "yuv frame size: " << yuv.cols << "x" << yuv.rows << std::endl;    

    return 0;
}