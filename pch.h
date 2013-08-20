#ifndef PCH_H
#define PCH_H

#include <QtCore>
#include <QtWidgets>
#include <QDebug>
#include <QtWebKit>
#include <QtWebKitWidgets/QWebView>

#include "util/mathutil.h"

#include <qdebug.h>
#define qTrace() QLOG_TRACE()
#define qDebug() QLOG_DEBUG()
#define qInfo() QLOG_INFO()
#define qWarning() QLOG_WARN()
#define qError() QLOG_ERROR()
#define qFatal() QLOG_FATAL()
#include "logger/QsLog.h"

#endif // PCH_H
