#ifndef CSERVER_H
#define CSERVER_H

#include <QObject>
#include <QDateTime>
#include <QWebSocket>
#include <QWebSocketServer>
#include "configuration.h"
#include "database.h"
#include "../sysinfo_api/packet.h"

class SysinfoServer : public QObject {
    Q_OBJECT
public:
    SysinfoServer(const QString &configPath, quint16 listenPort, QObject *parent = 0);

public slots:
    void onMessageReceived(const QString &msg);
    void onConnected();
    void onDisconnected();

signals:
    void closed();
    void onPacketReceived(const API::Packet& packet, const QDateTime& timestamp);
    void gotValidPacket();
    void gotRegisteredClient();

private:
    QWebSocketServer m_webSocket;
    QList<QWebSocket *> m_activeClients;
    Server::DataBase m_db;
    Server::Configuration m_config;

    void processPacket(const API::Packet &packet);
    void packetRequiresAlert(const API::Packet &packet, bool &cpu_alert, bool &memory_alert, bool &process_alert);
};

#endif // CServer_H
