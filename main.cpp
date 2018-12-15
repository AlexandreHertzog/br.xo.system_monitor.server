#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include "sysinfoserver.h"
#include "../sysinfo_api/exception.h"
#include "../sysinfo_api/log.h"

int main(int argc, char *argv[])
{
    qInstallMessageHandler(API::customLog);

    QCoreApplication a(argc, argv);

    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("SysInfo server: sysinfoserver");
    cmdParser.addHelpOption();

    QCommandLineOption portArg(QStringList() << "p" << "port",
                                  QCoreApplication::translate("main", "Port for server [default = 1234]."),
                                  QCoreApplication::translate("main", "port"), QLatin1Literal("1234"));
    QCommandLineOption configFileArg(QStringList() << "c" << "config",
                                     QCoreApplication::translate("main", "Configuration file for server [default = config.xml]."),
                                     QCoreApplication::translate("main", "config"), QLatin1Literal("config.xml"));
    cmdParser.addOption(portArg);
    cmdParser.addOption(configFileArg);
    cmdParser.process(a);

    int port = cmdParser.value(portArg).toInt();
    QString configFile = cmdParser.value(configFileArg);

    SysinfoServer* server;
    try
    {
        server = new SysinfoServer(configFile, port);
    }
    catch (const API::Exception& excep)
    {
        qCritical() << excep.message();
        return 1;
    }
    QObject::connect(server, &SysinfoServer::closed, &a, &QCoreApplication::quit);
    return a.exec();
}
