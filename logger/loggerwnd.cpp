#include "loggerwnd.h"
#include "ui_loggerwnd.h"
#include "QsLog.h"
#include <QFileDialog>
#include "../pch.h"

#ifdef PROFILE
QString ProfileDump();
#endif //PROFILE

LoggerWnd::LoggerWnd(QWidget *parent) :
    QWidget(parent),
	ui(new Ui::LoggerWnd)
{
	ui->setupUi(this);
	connect(ui->btnClear,SIGNAL(clicked()),this,SLOT(OnClear()));
	connect(ui->btnSaveAs,SIGNAL(clicked()),this,SLOT(OnSaveAs()));
	connect(ui->btnPerf,SIGNAL(clicked()),this,SLOT(OnProfile()));
	ui->cmbLevel->addItems(QStringList()<<"Trace"<<"Debug"<<"Info"<<"Warn"<<"Error"<<"Fatal");
	connect(ui->cmbLevel,SIGNAL(currentIndexChanged(int)),this,SLOT(OnLevelChanged(int)));
	QsLogging::Logger::instance().addDestination(this);
}

LoggerWnd::~LoggerWnd()
{
	QsLogging::Logger::instance().remDestination(this);
	delete ui;
}

void LoggerWnd::OnClear()
{
	ui->txtLog->clear();
}

void LoggerWnd::OnLevelChanged(int p_iLevel)
{
	QsLogging::Logger::instance().setLoggingLevel(QsLogging::Level(p_iLevel));
}

void LoggerWnd::OnProfile()
{
	QString sPerfLog="Profiling is not enabled in this build!";
#ifdef PROFILE
	sPerfLog=ProfileDump();
#endif //PROFILE
	QsLogging::Logger::instance().write(sPerfLog);
}

void LoggerWnd::OnSaveAs()
{
	QString sFile=QFileDialog::getSaveFileName(this,tr("Save Log File as ..."),QString(),"*.txt");
	if(sFile.isEmpty()) {return;};

	qDebug();
	QFile xFile(sFile);
	if(!xFile.open(QIODevice::WriteOnly)) {qError() << "Cannot save file:" << sFile;return;};
	QString sText=ui->txtLog->toPlainText();
	xFile.write(sText.toUtf8());
	qInfo() << "Log file written:" << sFile;
}

//void LoggerWnd::changeEvent(QEvent *p_xEvt)
//{
//    QWidget::changeEvent(p_xEvt);
//	switch (p_xEvt->type()) {
//	case QEvent::LanguageChange:
//		ui->retranslateUi(this);
//		break;
//	default:
//		break;
//	}
//}

//void LoggerWnd::resizeEvent(QResizeEvent* p_xEvt)
//{
//    QWidget::resizeEvent(p_xEvt);
//	ui->txtLog->resize(width(),height()-22);
//	ui->btnClear->move(width()-ui->btnClear->width(),0);
//    //qDebug() << "resizeEvent" << size();
//    OnResize();
//}

//void LoggerWnd::closeEvent(QCloseEvent* p_xEvt)
//{
//    QWidget::closeEvent(p_xEvt);
//	emit Closed();
//}

void LoggerWnd::OnResize()
{
    ui->centralwidget->resize(size());
    ui->txtLog->resize(width(),height()-22);
    ui->btnClear->move(width()-ui->btnClear->width(),0);
}

void LoggerWnd::write(const QString& p_sMsg)
{
	QString sType=p_sMsg.trimmed().split(" ")[0];
	QString sCol="#7f7f7f";
	//sType=sType.trimmed().toUpper();
	if(sType=="FATAL") {sCol="#ff0000";};
	if(sType=="ERROR") {sCol="#cf2020";};
	if(sType=="WARN") {sCol="#7f50ef";};
	if(sType=="INFO") {sCol="#efef20";};
	if(sType=="DEBUG") {sCol="#efefa0";};
	if(sType=="TRACE") {sCol="#efefef";};

	QString sMsg=p_sMsg;
	sMsg.replace("&","&amp;");
	sMsg.replace("\t","&nbsp;&nbsp;&nbsp;&nbsp;");
	sMsg.replace(" ","&nbsp;");
	sMsg.replace("<","&lt;");
	sMsg.replace(">","&gt;");
	sMsg.replace("\n","<br>");

	sMsg="<span style='color:"+sCol+";'>"+sMsg+"</span>";
	ui->txtLog->append(sMsg);
}
