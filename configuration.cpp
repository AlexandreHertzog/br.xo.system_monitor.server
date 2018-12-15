#include "configuration.h"
#include <QDebug>
#include <QFile>
#include "../sysinfo_api/exception.h"

namespace Server
{
Configuration::Configuration(const QString &path)
{
    QFile config(path);
    if (!config.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        API::Exception("Server configuration file not found").raise();
    }

    QXmlStreamReader xml_parser(&config);
    while (xml_parser.readNextStartElement())
    {
        if (xml_parser.name() == "client")
        {
            parseClientNode(xml_parser);
        }
    }
    if (m_clients.isEmpty())
    {
        API::Exception("Server configuration file did not contain valid clients").raise();
    }
}

bool Configuration::hasClient(int id) const
{
    return m_clients.find(id) != m_clients.end();
}

ClientInformation& Configuration::getClient(int id)
{
    auto clientIt = m_clients.find(id);
    if (clientIt == m_clients.end())
    {
        API::Exception("No client with id" + QString::number(id)).raise();
    }
    return clientIt.value();
}

void Configuration::parseClientNode(QXmlStreamReader& xml)
{
    Q_ASSERT(xml.name() == "client");

    auto attr = xml.attributes();
    if (!attr.hasAttribute("key") || !attr.hasAttribute("mail"))
    {
        qWarning() << "Client node does not have \'key\' or \'mail\' attributes.";
        return;
    }

    bool conversion_ok = true;
    const int client_id = attr.value("key").toInt(&conversion_ok);
    if (!conversion_ok)
    {
        qCritical() << "Invalid key value type in client description";
        return;
    }

    ClientInformation &info = m_clients[client_id];
    info.mail = attr.value("mail").toString();
    // Load sane defaults.
    info.cpu_limit = 80.0;
    info.memory_limit = 80.0;
    info.process_limit = 200;
    while (xml.readNextStartElement())
    {
        qDebug() << "At attribute" << xml.name();
        if (xml.name() == "alert")
        {
            parseAlert(info, xml);
        }
    }
}

void Configuration::parseAlert(ClientInformation &client, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.name() == "alert");

    auto alert = xml.attributes();
    if (!alert.hasAttribute("type") || !alert.hasAttribute("limit"))
    {
        qWarning() << "Found \'alert\' field with invalid attributes.";
        return;
    }
    // Required so that nextStartElement doesn't jump to the next 'client'.
    xml.skipCurrentElement();

    bool conversion_ok = true;

    QString alert_type = alert.value("type").toString();
    QString alert_limit = alert.value("limit").toString();
    // 'memory' and 'cpu' values end with a '%'. Since it is the only non-digit character
    // allowed, it is discarded and ignored in an optional fashion.
    if (alert_type == "memory")
    {
        if (alert_limit.endsWith('%'))
        {
            alert_limit.chop(1);
        }
        client.memory_limit = alert_limit.toInt(&conversion_ok);
    }
    else if (alert_type == "cpu")
    {
        if (alert_limit.endsWith('%'))
        {
            alert_limit.chop(1);
        }
        client.cpu_limit = alert_limit.toInt(&conversion_ok);
    }
    else if (alert_type == "processes")
    {
        client.process_limit = alert_limit.toInt(&conversion_ok);
    }
    else
    {
        qWarning() << "Unrecognized field" << alert_type << "in client structure";
        return;
    }
    if (!conversion_ok)
    {
        qWarning() << "Invalid data type in client definition";
    }
}

} // namespace Server
