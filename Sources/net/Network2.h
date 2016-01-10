#pragma once

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QTextCodec>
#include <QJsonObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "NetworkMessagesTypes.h"

struct Message2
{
    qint32 type;
    QString json;
};

Q_DECLARE_METATYPE(Message2)

class Network2;

class SocketHandler: public QObject
{
    Q_OBJECT
public:
    SocketHandler(Network2* network);
public slots:
    void process();

    void tryConnect(QString host, int port, QString login, QString password);
    void socketConnected();

    void sendMessage(Message2 message);
    void disconnectSocket();
    void errorSocket(QAbstractSocket::SocketError error);
    void handleFirstMessage();
    void handleNewData();
signals:
    void firstMessage();
    void connectionEnd(QString reason);
    void readyToStart(int your_id, QString map);
private:
    void HandleSuccessConnection(Message2 m);

    QTcpSocket socket_;

    void SendData(const QByteArray& data);

    QTextCodec* net_codec_;

    QString login_;
    QString password_;

    QString map_;
    int your_id_;

    bool is_first_message_;

    qint32 message_size_;
    qint32 message_type_;

    QByteArray buffer_;

    enum ReadingState
    {
        HEADER, BODY
    } reading_state_;

    enum NetworkState
    {
        NOT_CONNECTED, CONNECTING, CONNECTED, DISCONNECTED
    };
    NetworkState state_;

    void HandleHeader();
    void HandleBody();

    QString possible_error_reason_;

    Network2* network_;
};

class Network2: public QObject
{
    Q_OBJECT
public:
    friend class SocketHandler;

    static QJsonObject ParseJson(Message2 message);

    static bool IsKey(const QJsonObject& json, const std::string& key);
    static size_t ExtractObjId(const QJsonObject& json);

    static Network2& GetInstance();

    bool IsGood();

    void SendMap(QString url, QByteArray data);

    void TryConnect(QString host, int port, QString login, QString password);

    void SendMsg(Message2 message);

    void SendOrdinaryMessage(QString text);

    void SendPing(QString ping_id);

    void Disconnect();

    bool IsMessageAvailable();
    Message2 PopMessage();

    QByteArray GetMapData();
public slots:
    void onConnectionEnd(QString reason);
signals:
    void connectRequested(QString host, int port, QString login, QString password);
    void sendMessage(Message2 message);
    void disconnectRequested();
    void connectionSuccess(int your_id, QString map);
    void connectionFailed(QString reason);
private slots:
    void mapDownloaded(QNetworkReply* reply);
    void downloadMap(int your_id, QString map);
private:
    bool is_good_;

    int your_id_;
    QString map_url_;

    void PushMessage(Message2 message);

    QMutex queue_mutex_;
    QQueue<Message2> received_messages_;

    Network2();

    SocketHandler handler_;
    QThread thread_;

    QNetworkAccessManager* net_manager_;
    QByteArray map_data_;
};