#ifndef CDATABASE_H
#define CDATABASE_H

#include <QObject>
#include <QDateTime>
#include <QFile>
#include <QXmlStreamWriter>
#include "../sysinfo_api/packet.h"

namespace Server
{
// Really simple front-end to the database, which is a XML file.
class DataBase : public QObject
{
    Q_OBJECT
public:
    DataBase(const QString &path);
    ~DataBase();

public slots:
    void put(const API::Packet &packet, const QDateTime &timestamp);

private:
    QFile m_dbFile;
    QXmlStreamWriter m_xmlWriter;
};
} // namespace Server

#endif // CDATABASE_H
