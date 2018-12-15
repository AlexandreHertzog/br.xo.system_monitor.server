#include "database.h"
#include <QFile>
#include <QXmlStreamWriter>
#include <QThread>
#include <QDebug>
#include <unistd.h>
#include "../sysinfo_api/exception.h"

namespace Server
{
DataBase::DataBase(const QString &path) :
    m_dbFile(path)
{
    if (!m_dbFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        API::Exception("Failed to open database file.").raise();
    }
    m_xmlWriter.setDevice(&m_dbFile);
    m_xmlWriter.setAutoFormatting(true);
    m_xmlWriter.writeStartDocument();
}

DataBase::~DataBase()
{
}

void DataBase::put(const API::Packet &packet, const QDateTime &timestamp)
{
    m_xmlWriter.writeStartElement("client");
    m_xmlWriter.writeAttribute("id", QString::number(packet.getId()));
    m_xmlWriter.writeAttribute("timestamp", timestamp.toString("yyyy/MM/dd hh:mm:ss"));
    m_xmlWriter.writeTextElement("cpuLoad", QString::number(packet.getCPULoad()));
    m_xmlWriter.writeTextElement("memLoad", QString::number(packet.getMemoryLoad()));
    m_xmlWriter.writeTextElement("procCount", QString::number(packet.getNumProcesses()));
    m_xmlWriter.writeEndElement();
    if (!m_dbFile.flush())
    {
        qWarning() << "Failed to write data to DB";
    }
}
} // namespace Server
