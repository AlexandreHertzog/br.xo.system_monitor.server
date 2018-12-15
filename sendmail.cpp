#include "sendmail.h"

#include <QProcess>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QDebug>

#include <stdio.h>

#include "../sysinfo_api/exception.h"

static const QString newline = "\n";

namespace Server
{
QString SendMail::SMail::toText()
{
    QString text;
    QTextStream textStream(&text);
    textStream << "To: " << receiver << newline;
    textStream << "From: " << sender << newline;
    textStream << "Subject: " << subject << newline << newline;
    textStream << message << newline;
    return text;
}

SendMail::SendMail(QObject *parent) : QObject(parent)
{

}

void SendMail::send(const SMail &mail)
{
    qInfo() << "Sending mail to" << mail.receiver;
    // Using basic structures and functions here since QFile and QTextStream
    // could not manage to pipe the message to sendmail correctly. This happens
    // because QTextStream somehow messes '\n' up, making commands very hard to
    // debug.
    // Due to time constraints, this simple solution was adopted.
    FILE *mailFile = popen("/usr/lib/sendmail -t", "w");
    if (mailFile != nullptr)
    {
        fprintf(mailFile, "To: %s\n", mail.receiver.toLocal8Bit().constData());
        fprintf(mailFile, "From: %s\n", mail.sender.toLocal8Bit().constData());
        fprintf(mailFile, "Subject: %s\n", mail.subject.toLocal8Bit().constData());
        fwrite(mail.message.toLocal8Bit().constData(), 1, mail.message.length(), mailFile);
        fwrite(".\n", 1, 2, mailFile);
        pclose(mailFile);
    }
}
} // namespace Server
