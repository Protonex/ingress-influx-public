#ifndef LOGGERWND_H
#define LOGGERWND_H

#include <QtGui>
#include "QsLogDest.h"
#include <QWidget>

namespace Ui {
class LoggerWnd;
}

class LoggerWnd : public QWidget, QsLogging::Destination
{
	Q_OBJECT
	
public:
	explicit LoggerWnd(QWidget *parent = 0);
	~LoggerWnd();

signals:
	void Closed();

public slots:
	void OnClear();
	void OnSaveAs();
	void OnProfile();
	void OnLevelChanged(int p_iLevel);
    void OnResize();
	
protected:
    //void changeEvent(QEvent* p_xEvt);
    //void resizeEvent(QResizeEvent* p_xEvt);
    //void closeEvent(QCloseEvent* p_xEvt);
	void write(const QString& p_sMsg);
	
private:
	Ui::LoggerWnd *ui;
};

#endif // LOGGERWND_H
