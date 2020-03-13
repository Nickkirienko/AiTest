#ifndef SPLITTER_H
#define SPLITTER_H
#include <opencv2/opencv.hpp>

class Splitter
{
public:
    Splitter();
    Splitter(int splitWidth, int splitHeight, int overlay);
    std::vector<std::vector<cv::Mat>> split(cv::Mat img);
    cv::Mat merge(std::vector<std::vector<cv::Mat>>);    
private:
    int overlay = 100;
    int splitWidth = 1400;
    int splitHeight = 1400;

};

#endif // SPLITTER_H
