#include "sysinfoserver.h"
#include <QJsonDocument>
#include <QJsonObject>

#include "../sysinfo_api/packet.h"
#include "../sysinfo_api/exception.h"
#include "sendmail.h"

SysinfoServer::SysinfoServer(const QString &configPath, quint16 listenPort, QObject *parent) : QObject(parent),
    m_webSocket("SysinfoServer", QWebSocketServer::NonSecureMode, this),
    m_db("sysinfo_db.xml"),
    m_config(configPath)
{
    if (!m_webSocket.listen(QHostAddress::Any, listenPort))
    {
        API::Exception("Failed to open a listen socket").raise();
    }
    qInfo() << "SysinfoServer is listening on port" << listenPort;
    connect(&m_webSocket, &QWebSocketServer::newConnection, this, &SysinfoServer::onConnected);
    connect(this, &SysinfoServer::onPacketReceived, &m_db, &Server::DataBase::put);
}

void SysinfoServer::onConnected()
{
    qInfo() << "Client connected";

    QWebSocket *clientSocket = m_webSocket.nextPendingConnection();
    if (clientSocket)
    {
        connect(clientSocket, &QWebSocket::textMessageReceived, this, &SysinfoServer::onMessageReceived);
        connect(clientSocket, &QWebSocket::disconnected, this, &SysinfoServer::onDisconnected);
        m_activeClients << clientSocket;
    }
}

void SysinfoServer::onMessageReceived(const QString &msg)
{
    qDebug() << "Receiving message:" << msg;

    API::Packet packet;
    try
    {
        QJsonDocument json_doc = QJsonDocument::fromJson(msg.toUtf8());
        packet.parse(json_doc.object());
    }
    catch (API::Exception& except)
    {
        qCritical() << except.message() << "on frame receiving";
        return;
    }
    emit gotValidPacket();
    if (!m_config.hasClient(packet.getId()))
    {
        qWarning() << "Did not find client" << packet.getId();
        return;
    }
    processPacket(packet);
    emit gotRegisteredClient();
}

void SysinfoServer::onDisconnected()
{
    qInfo() << "Client disconnected";
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (client)
    {
        m_activeClients.removeAll(client);
        client->deleteLater();
    }
}

void SysinfoServer::processPacket(const API::Packet &packet)
{
    emit onPacketReceived(packet, QDateTime::currentDateTime());

    bool cpu_alert = false, memory_alert = false, process_alert = false;
    packetRequiresAlert(packet, cpu_alert, memory_alert, process_alert);
    if (!cpu_alert && !memory_alert && !process_alert)
    {
        return;
    }

    const QString client_id = QString::number(packet.getId());

    Server::SendMail::SMail mail;
    //mail.sender = "client_reports@127.0.0.1";
    mail.sender = m_config.getClient(packet.getId()).mail;
    mail.receiver = m_config.getClient(packet.getId()).mail;
    mail.subject = "Report for client " + client_id;
    mail.message = "This is an automatic message. You are receiving this because you are registered ";
    mail.message += "in the client " + client_id + " in our monitoring system. ";
    mail.message += "The client sounded the alarms for:";
    if (cpu_alert)
    {
        mail.message += "\n\t * The CPU load is above the limit.";
    }
    if (memory_alert)
    {
        mail.message += "\n\t * The memory load is above the limit.";
    }
    if (process_alert)
    {
        mail.message += "\n\t * The proccess count is above the limit.";
    }
    try
    {
        Server::SendMail(this).send(std::move(mail));
    }
    catch (API::Exception &except)
    {
        qCritical() << except.message();
    }
}

void SysinfoServer::packetRequiresAlert(const API::Packet &packet, bool &cpu_alert, bool &mem_alert, bool &process_alert)
{
    cpu_alert = mem_alert = process_alert = false;

    const Server::ClientInformation& info = m_config.getClient(packet.getId());
    if (info.cpu_limit <= packet.getCPULoad())
    {
        cpu_alert = true;
    }
    if (info.memory_limit <= packet.getMemoryLoad())
    {
        mem_alert = true;
    }
    if (info.process_limit <= packet.getNumProcesses())
    {
        process_alert = true;
    }
}
