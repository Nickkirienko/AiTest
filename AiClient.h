#ifndef AICLIENT_H
#define AICLIENT_H
#include <string>
#include <QUrl>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QUrlQuery>
#include <QCoreApplication>
#include <QProcess>
#include <thread>
#include <mutex>
#include <QFuture>

class AiClient : public QObject
{
    Q_OBJECT
public:
    explicit AiClient(QObject *parent = nullptr);
    cv::Mat protsessImage(cv::Mat img_gpu);

private:

    struct AiStackItem
    {
        int itmId;
        std::vector<uchar> dataBuffer;
        QByteArray replyData;
        bool ready = false;
        bool error = false;
    };

    std::string m_ip = "0.0.0.0";
    int port = 8088;
    bool m_error = false;
    QString m_aiClientLocation = "/home/darkone/Desktop/OptiAnalyzer/OptiAi/optiAIApi.py";
    QString m_pythonLocation = "/home/darkone/anaconda3/bin/python";
    int m_aiLoadingTimeout = 1;
    QNetworkRequest m_request;
    std::mutex m_mtx;
    std::list<AiStackItem> m_taskStack;
    QFuture<void> m_runThread;
    QProcess m_aiProcess;

    void runClient();
    std::string isAiRunning();
private slots:
};


#endif // R3NETCLIENT_H
