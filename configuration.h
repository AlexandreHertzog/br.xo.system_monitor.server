#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QMap>
#include <QXmlStreamReader>

namespace Server
{
struct ClientInformation
{
    QString mail;
    unsigned memory_limit;
    unsigned cpu_limit;
    unsigned process_limit;
};

// Holds the server configuration, mainly doing:
//  * Reading client data from XML file
//  * Getting client information (as in the above structure)
class Configuration
{
public:
    explicit Configuration(const QString &path);
    bool hasClient(int id) const;
    ClientInformation& getClient(int id);

private:
    QMap<int, ClientInformation> m_clients;

    void parseClientNode(QXmlStreamReader& xml);
    void parseAlert(ClientInformation &client, QXmlStreamReader &xml);
};
} // namespace Server
#endif // CONFIGURATION_H
