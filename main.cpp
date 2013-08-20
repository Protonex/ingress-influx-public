#include "influxwnd.h"
#include <QApplication>
#include "logger/QsLog.h"
#include "logger/QsLogDest.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // init the logging mechanism
    QsLogging::Logger& logger = QsLogging::Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    const QString sLogPath(QDir::homePath()+"/Influx/log.txt");
    QsLogging::DestinationPtr fileDestination(QsLogging::DestinationFactory::MakeFileDestination(sLogPath));
    QsLogging::DestinationPtr debugDestination(QsLogging::DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(debugDestination.get());
    logger.addDestination(fileDestination.get());
    //logger.setLoggingLevel(QsLogging::InfoLevel);


    InfluxWnd w;
    w.show();
    
    return a.exec();
}
