#include "splitter.h"

Splitter::Splitter(int splitWidth, int splitHeight, int overlay)
{
    this->splitWidth = splitWidth;
    this->splitHeight = splitHeight;
    this->overlay = overlay;
}

std::vector<std::vector<cv::Mat>> Splitter::split(cv::Mat img)
{
    std::vector<std::vector<cv::Mat>> splitted;
    std::vector<cv::Mat> hor;
    int n1 = ceil((double)img.cols/(double)splitWidth);
    int w = img.cols/n1;
    int n2 = ceil((double)img.rows/(double)splitHeight);
    int h = img.rows/n2;
    std::cout << "Splitting image into: " << n1*n2 << std::endl;
    if(n1 > 1){
        for(int i = 1; i < n1+1; i++){
            cv::Mat im;
            if(i == 1){
                im = img(cv::Rect(0,0, img.cols/n1*i+overlay, img.rows)).clone();
            }
            else if(i == n1){
                im = img(cv::Rect(w*(i-1)-overlay,0, img.cols/n1+overlay, img.rows)).clone();
            }
            else{
                im = img(cv::Rect(w*(i-1)-overlay,0, img.cols/n1+overlay*2, img.rows)).clone();
            }
            hor.push_back(im);
        }
    }
    else{
        hor.push_back(img);
    }
    if(n2 > 1){        
        for(int j = 0; j < hor.size(); j++){
            std::vector<cv::Mat> ver;
            cv::Mat ihor = hor[j];            
            for(int i = 1; i < n2+1; i++){
                cv::Mat im;
                if(i == 1){
                    im = ihor(cv::Rect(0,0, ihor.cols, ihor.rows/n2*i+overlay)).clone();
                }
                else if(i == n2){
                    im = ihor(cv::Rect(0, h*(i-1)-overlay, ihor.cols, ihor.rows/n2+overlay)).clone();
                }
                else{
                    im = ihor(cv::Rect(0, h*(i-1)-overlay, ihor.cols, ihor.rows/n2+overlay*2)).clone();
                }
                ver.push_back(im);
            }
            splitted.push_back(ver);
        }        
    }
    else{
        for(int i = 0; i < hor.size(); i++){
            std::vector<cv::Mat> ver;
            ver.push_back(hor[i]);
            splitted.push_back(ver);
        }
    }
    return splitted;
}

cv::Mat Splitter::merge(std::vector<std::vector<cv::Mat>> splitted)
{
    cv::Mat res;
    for(int i = 0; i < splitted.size(); i++){
        cv::Mat h;
        std::vector<cv::Mat> hor = splitted[i];
        if(hor.size() == 1){
           h = hor[0];
        }
        else{
            for(int j = 0; j < hor.size(); j++){
                cv::Mat im = hor[j];
                if(j == 0){
                    h = im(cv::Rect(0,0, im.cols, im.rows-overlay)).clone();
                }
                else if(j == hor.size()-1){
                    cv::vconcat(h,im(cv::Rect(0, overlay, im.cols, im.rows-overlay)).clone(),h);
                }
                else{
                    cv::vconcat(h,im(cv::Rect(0, overlay, im.cols, im.rows-overlay*2)).clone(),h);
                }
            }
        }
        if(splitted.size() == 1){
            res = h;
        }
        else{
            if(i == 0){
                res = h(cv::Rect(0,0, h.cols-overlay, h.rows)).clone();
            }
            else if(i == splitted.size()-1){
                cv::hconcat(res,h(cv::Rect(overlay, 0, h.cols-overlay, h.rows)).clone(),res);
            }
            else{
                cv::hconcat(res,h(cv::Rect(overlay, 0, h.cols-overlay*2, h.rows)).clone(),res);
            }
        }
    }
    std::cout << "Images merged" << std::endl;
    return res;
}
