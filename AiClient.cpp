#include "AiClient.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <QtConcurrent/QtConcurrent>

using namespace std;
using namespace cv;

static bool aiLoaded = false;

AiClient::AiClient(QObject *parent):
    QObject(parent)
{
    if (!aiLoaded)
    {
    QStringList arguments {m_aiClientLocation};
    m_aiProcess.start(m_pythonLocation, arguments);

    //wait for ai to boot up
    int loadTime = 0;
    string error;

    while (loadTime < m_aiLoadingTimeout)
    {
        error = isAiRunning();
        if (error == "") break;
        this_thread::sleep_for(chrono::seconds(1));
        m_aiLoadingTimeout++;
    }

    if (loadTime >=  m_aiLoadingTimeout)
    {
        cout << error << endl;
        cout << "Error Loading AI" << endl;       
    }

    aiLoaded = true;
    m_runThread = QtConcurrent::run(this, &AiClient::runClient);
    cout << "AI loaded" << endl;    
    }
    srand(time(NULL));
}

Mat AiClient::protsessImage(Mat img_gpu)
{
    Mat out_cpu;

    if (!aiLoaded)
    {
        cout << "Error AI is not running" << endl;       
    }
    AiStackItem stackItem;
    stackItem.itmId = rand() % INT_MAX + 1;

    std::vector<uchar> buffer;
    imencode(".png", img_gpu, stackItem.dataBuffer);

    {
        unique_lock<std::mutex> lck(m_mtx);
        m_taskStack.push_back(stackItem);
    }

    while(m_taskStack.front().itmId != stackItem.itmId ||
          !m_taskStack.front().ready)
    {
        this_thread::sleep_for(chrono::milliseconds(1));
    }

    AiStackItem aiRes;
    {
        unique_lock<std::mutex> lck(m_mtx);
        stackItem = m_taskStack.front();
        m_taskStack.pop_front();
    }

    if (stackItem.error)
    {
        std::cout << "Client Error " << endl;
    }

    char * repData = stackItem.replyData.data();
    out_cpu = Mat(img_gpu.rows, img_gpu.cols, CV_8UC1);
    memcpy(out_cpu.data, repData,out_cpu.rows * out_cpu.cols);
    return out_cpu;
}

string AiClient::isAiRunning()
{
    //check if ai is running
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QString urlSring = "http://" + QString::fromStdString(m_ip);
    urlSring += ":" + QString::number(port) + "/ready";
    QUrl m_url(urlSring);
    m_request = QNetworkRequest(m_url);

    QNetworkReply *reply = manager->get(m_request);

    while (!reply->isFinished())
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }

    if (reply->error())
    {
        return reply->errorString().toStdString();
    }

    auto replyBytes = reply->readAll();
    QString replyString = QString(replyBytes);

    if ("ready" == replyString)
    {
        return "";
    }
    else {
        return replyString.toStdString();
    }
}

void AiClient::runClient()
{
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QString urlSring = "http://" + QString::fromStdString(m_ip);
    urlSring += ":" + QString::number(port) + "/predict";
    QUrl m_url(urlSring);
    m_request = QNetworkRequest(m_url);

    while (true)
    {
        if (m_taskStack.empty() ||
            m_taskStack.front().ready)
        {
            this_thread::sleep_for(chrono::milliseconds(1));
            continue;
        }

        QHttpMultiPart *httpMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        QHttpPart fileDataPart;
        fileDataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\"; filename=\"bxvcvbvcbvcx_0_0.jpg\""));
        fileDataPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
        fileDataPart.setBody(QByteArray::fromRawData((char*)m_taskStack.front().dataBuffer.data(), m_taskStack.front().dataBuffer.size()));
        httpMultiPart->append(fileDataPart);
        QNetworkReply *reply = manager->post(m_request, httpMultiPart);

        while (!reply->isFinished())
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        }

        unique_lock<std::mutex> lck(m_mtx);
        m_taskStack.front().ready = true;

        if (reply->error())
        {
            m_taskStack.front().error = true;
            continue;
        }
        m_taskStack.front().replyData = reply->readAll();
    }
}
