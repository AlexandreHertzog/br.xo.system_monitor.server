#ifndef CSENDMAIL_H
#define CSENDMAIL_H

#include <QObject>

namespace Server
{
// Interfaces with the command line to use the "sendmail" Linux command.
// It pipes text to that command, which effectively encapsulates and sends
// the e-mail.
class SendMail : public QObject
{
    Q_OBJECT
public:
    struct SMail
    {
        QString sender;
        QString receiver;
        QString subject;
        QString message;

        QString toText();
    };

    explicit SendMail(QObject *parent = 0);

public slots:
    void send(const SMail &mail);
};
} // namespace Server

#endif // CSENDMAIL_H
