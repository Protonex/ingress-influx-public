#ifndef QWEBVIEWEX_H
#define QWEBVIEWEX_H

#include <QWebView>

class QWebViewEx : public QWebView
{
	Q_OBJECT
public:
	explicit QWebViewEx(QWidget *parent = 0);
	
	enum ContextMenuBehaviour
	{
		CMB_Default,
		CMB_Small, //link-copy, back,reload
		CMB_None,
	};

	ContextMenuBehaviour m_eContextMenuBehaviour;

signals:
	
public slots:

protected:
	QWebView* createWindow(QWebPage::WebWindowType type);
	void contextMenuEvent(QContextMenuEvent *event);
	void dragEnterEvent(QDragEnterEvent *p_xE);
	void dropEvent(QDropEvent*p_xE);
};

class QWebPageEx : public QWebPage
{
	Q_OBJECT
public:
	explicit QWebPageEx(QWidget *parent = 0);

	enum NewWindowBehaviour
	{
		NWB_Default,
		NWB_OpenWebView,
		NWB_SystemBrowser,
	};

	QUrl m_xRequestedURL;
	NewWindowBehaviour m_eNewWindowBehaviour;

signals:

public slots:

protected:
	QWebPage *createWindow(WebWindowType type);
	bool acceptNavigationRequest(QWebFrame* frame,const QNetworkRequest& request,NavigationType type);
};

#endif // QWEBVIEWEX_H
