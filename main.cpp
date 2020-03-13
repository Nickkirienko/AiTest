#include <QCoreApplication>
#include <opencv2/opencv.hpp>
#include "opencv2/objdetect.hpp"
#include <iostream>
#include <QDir>
#include "AiClient.h"
#include "splitter.h"

using namespace cv;
using namespace std;

static int nrOfClasses = 4;
static string inPath = "/home/nick/AI/";
static string outPath = "/home/nick/AI/out/";
static string testPath = "/home/nick/AI/testAi/";
static int randiff = 8;
static int MAX_SIZE = 3000000;

struct AnalizeType{
    int id;
    std::string name;
    cv::Scalar color;
};

const AnalizeType analizeTypes[] =
{
    {1,"Dots", cv::Scalar(0,0,255)},
    {2,"Scratches", cv::Scalar(0,255,0)},
    {3,"Discoloration", cv::Scalar(0,255,255)},
    {4,"Cracks", cv::Scalar(255,0,0)}
};

Mat convertValImg(Mat img);
void createFolders();
void convertImgs();
void testOut();
void testAi();

uchar classMap[][4]=
{
    {1,1},
    {2,2},
    {3,3},
    {4,2},
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    createFolders();
    convertImgs();
    testOut();
    testAi();
    return a.exec();
}

void convertImgs()
{    
    QStringList fullFilePath;
    QDir inDir(QString::fromStdString(inPath + "/img/" ));
    QDir directory(inDir);
    QStringList trainImgs = directory.entryList(QStringList() << "*.png" << "*.jpg",QDir::Files);

    srand (time(NULL));

    if (trainImgs.count() == 0)
    {
        cout << "trainImgs.count() == 0" << endl;
        exit(0);
    }

    cout << "files found:"  << trainImgs.count() << endl;
    createFolders();

    ofstream trainFile;
    ofstream trainvalFile;
    ofstream valFile;

    trainFile.open (outPath + "train.txt");
    valFile.open (outPath + "val.txt");
    trainvalFile.open (outPath + "trainval.txt");

    int MaxH = 0;
    int MaxW = 0;

    foreach(QString filename, trainImgs) {
        QString temp = filename;
        QString imgNameQ = temp.replace(".jpg", "");
        imgNameQ = imgNameQ.replace(".JPG", "");
        imgNameQ = imgNameQ.replace(".png", "");

        Mat trainImg = imread(inPath  + "/img/"+ filename.toStdString());

        if (trainImg.empty())
        {   cout << filename.toStdString() << endl;
            cout << "trainImg.empty()" << endl;
            continue;
        }

        QString segFile = imgNameQ + ".png";
        Mat segImg = imread(inPath  + "/mask/" + segFile.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);

        if (segImg.empty())
        {
            cout << inPath + segFile.toStdString() << endl;
            cout << "segImg.empty()" << endl;
            continue;
        }

        if (trainImg.rows != segImg.rows ||
                trainImg.cols != segImg.cols)
        {
            cout << filename.toStdString() << endl;
            cout << "rainImg.rows != segImg.rows || trainImg.cols != segImg.cols" << endl;
            continue;
        }

        int imgsize = segImg.rows * segImg.cols;

        if (segImg.rows > MaxH)
        {
            MaxH = segImg.rows;
        }
        if (segImg.cols > MaxW)
        {
            MaxW = segImg.cols;
        }

        imgNameQ = QString::number(rand() % 9000000000000);

        Mat segImgNew = convertValImg(segImg);
        if (rand() % randiff == 0)
        {
            trainvalFile << imgNameQ.toStdString() << endl;
            valFile << imgNameQ.toStdString() << endl;
        }
        else
        {
            trainvalFile << imgNameQ.toStdString() << endl;
            trainFile << imgNameQ.toStdString() << endl;
        }
        string savePathMask = outPath + "/mask/"+ imgNameQ.toStdString() + ".png";
        string savePathImg = outPath + "/img/"+ imgNameQ.toStdString() + ".png";
        //trainImg = trainImg(cv::Rect(0,0, 1000, 1000)).clone();
        //segImgNew = segImgNew(cv::Rect(0,0, 1000, 1000)).clone();
        imwrite(savePathImg,trainImg);
        imwrite(savePathMask,segImgNew);
    }

    valFile.close();
    trainFile.close();

    cout << "Max h:" << MaxH << " max w:" << MaxW << endl;
    cout << "Done" << endl;
}

void testOut(){
    QDir inDirOrig(QString::fromStdString(outPath + "img/"));
    QStringList trainImgs = inDirOrig.entryList(QStringList() << "*.png",QDir::Files);

    QDir dirTemp(QString::fromStdString(outPath));
    if (!dirTemp.exists())
        dirTemp.mkpath(".");

    for (auto itm : trainImgs)
    {
        QString temp = itm;
        QString imgNameQ = temp.replace(".png", "");
        imgNameQ = imgNameQ.replace(".JPG", "");
        string imgNameS = imgNameQ.toStdString();

        cout << "Makeing image:"  <<  imgNameS << endl;

        Mat trainImg = imread(inPath + "img/" + imgNameS + ".png" );
        Mat segImgNew = imread(inPath + "mask/" + imgNameS + ".png", CV_LOAD_IMAGE_GRAYSCALE);
        Mat outImg = trainImg.clone();

        Mat classImg[4];

        for (int i = 0; i < 4; i++)
        {
            classImg[i] = Mat::zeros(trainImg.rows, trainImg.cols, CV_8UC1);
        }

        for (auto i = 0; i < trainImg.rows *  trainImg.cols; i++)
        {
            uchar val = segImgNew.data[i];

            if (val == 0 || val > 4) continue;
            classImg[val-1].data[i] = 255;
        }

        for (int i = 0 ; i < 4 ; i++)
        {
            auto color = analizeTypes[i].color;
            std::vector<vector<Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            findContours(classImg[i], contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
            drawContours(outImg, contours, -1, color);
        }
        string fileFullOut = outPath + imgNameS + ".jpg";
        imwrite(fileFullOut,outImg);
    }
}

void testAi(){
    QDir directory(QString::fromStdString(inPath + "/img"));
    QStringList trainImgs = directory.entryList(QStringList() << "*.png" << "*.JPG" << "*.PNG" << "*.jpg",QDir::Files);
    AiClient aiClient;

    if (trainImgs.count() == 0)
    {
        cout << "trainImgs.count() == 0" << endl;
        return;
    }
    auto start = chrono::high_resolution_clock::now();
    for(QString filename: trainImgs)
    {
        QString temp = filename;
        QString imgNameQ = temp.replace(".jpg", "");
        imgNameQ = imgNameQ.replace(".JPG", "");
        imgNameQ = imgNameQ.replace(".png", "");
        imgNameQ = imgNameQ.replace(".PNG", "");

        Mat origImg = imread(inPath + filename.toStdString());
        if (origImg.empty())
        {   cout << imgNameQ.toStdString() << endl;
            cout << "trainImg.empty()" << endl;
            continue;
        }
        Splitter splitter(1000,1000,100);
        std::vector<std::vector<Mat>> splitted = splitter.split(origImg);
        std::vector<std::vector<Mat>> processed;
        for(std::vector<Mat> vec : splitted){
            std::vector<Mat> hor;
            for(Mat im : vec){
                hor.push_back(aiClient.protsessImage(im));
            }
            processed.push_back(hor);
        }
        Mat resImg = splitter.merge(processed);

        Mat tempImg[3];
        for (int i = 0; i < 3; i++)
        {
            tempImg[i] = Mat::zeros(resImg.rows, resImg.cols, CV_8UC1);
        }

        for (int i = 0 ; i < resImg.rows*resImg.cols; i++ )
        {
            uchar val = resImg.data[i];

            if (val)
            {
                tempImg[val-1].data[i] = 255;
            }
        }
        //origImg = Mat::zeros(origImg.rows, origImg.cols, CV_8UC3 );
        for (int i = 0; i < 3; i++)
        {
            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            findContours( tempImg[i], contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
            cv::drawContours(origImg, contours, -1, Scalar(classMap[i][3], classMap[i][2], classMap[i][1]));
        }

        imwrite(outPath + imgNameQ.toStdString() + "_mask.jpg" ,origImg);
    }
    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = finish - start;
    cout << "Processing time: " << elapsed.count() << endl;
}

void createFolders()
{
    QString outDirQ = QString::fromStdString(outPath);
    {
        QDir dir(outDirQ + "/img");
        if (!dir.exists())
            dir.mkpath(".");
    }
    {
        QDir dir(outDirQ + "/mask");
        if (!dir.exists())
            dir.mkpath(".");
    }
}

Mat convertValImg(Mat img)
{
    Mat outImg = Mat::zeros(img.rows, img.cols, CV_8UC1);
    for (int i = 0; i < img.rows * img.cols; i++)
    {
        int inIndex = i ;
        uchar val = img.data[inIndex];
        bool found = false;

        for (int j = 0; j < nrOfClasses; j++)
        {
            if (classMap[j][0] == val )
            {
                outImg.data[i] = classMap[j][1];
                found = true;
            }
        }
    }
    return outImg;
}
