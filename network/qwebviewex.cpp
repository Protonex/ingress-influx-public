#include "../pch.h"
#include "qwebviewex.h"
#include <QDesktopServices>
#include <QDropEvent>
#include <QtWebKitWidgets/QWebFrame>

QWebViewEx::QWebViewEx(QWidget *parent) :
	QWebView(parent)
{
	m_eContextMenuBehaviour=CMB_Small;
}

QWebView* QWebViewEx::createWindow(QWebPage::WebWindowType type)
{
	//qDebug() << "CW:" << type;
	return QWebView::createWindow(type);
}

void QWebViewEx::contextMenuEvent(QContextMenuEvent *event)
{
	QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());

	if(m_eContextMenuBehaviour==CMB_Small)
	{
		if (!r.linkUrl().isEmpty()) {
			QMenu menu(this);
			menu.addAction(pageAction(QWebPage::CopyLinkToClipboard));
			menu.exec(mapToGlobal(event->pos()));
			return;
		}
		else
		{
			QMenu menu(this);
			QAction* pxActBack=pageAction(QWebPage::Back);
			QAction* pxActReload=pageAction(QWebPage::Reload);
			pxActBack->setIcon(QIcon());
			pxActReload->setIcon(QIcon());

			menu.addAction(pxActBack);
			menu.addAction(pxActReload);
			menu.exec(mapToGlobal(event->pos()));
			return;
		};
	};
	if(m_eContextMenuBehaviour==CMB_Default)
	{
		QWebView::contextMenuEvent(event);
	};
}

void QWebViewEx::dropEvent(QDropEvent *p_xE)
{
	//p_xE->acceptProposedAction();
}

void QWebViewEx::dragEnterEvent(QDragEnterEvent *p_xE)
{
	//p_xE->acceptProposedAction();
}


/////////////////////////////////////////////////////////////////////////////

QWebPageEx::QWebPageEx(QWidget *parent) :
	QWebPage(parent)
{
	m_eNewWindowBehaviour=NWB_Default;
}

QWebPage* QWebPageEx::createWindow(QWebPage::WebWindowType type)
{
	//qDebug() << "CWp:" << type;
	if(m_eNewWindowBehaviour==NWB_SystemBrowser)
	{
		QDesktopServices::openUrl(m_xRequestedURL);
		return NULL;
	};
	if(m_eNewWindowBehaviour==NWB_OpenWebView)
	{
		QWebView *webView = new QWebView;
		QWebPage *newWeb = new QWebPage(webView);
		webView->setAttribute(Qt::WA_DeleteOnClose, true);
		webView->setPage(newWeb);
		webView->show();
		return newWeb;
	};

	return QWebPage::createWindow(type);
}

bool QWebPageEx::acceptNavigationRequest(QWebFrame* frame,const QNetworkRequest& request,NavigationType type)
{
	m_xRequestedURL=request.url();
	//qDebug() << "ANR:" << request.url() << type;
	//scripted mailto button?
	if(type==QWebPage::NavigationTypeOther&&m_xRequestedURL.toString().startsWith("mailto:"))
	{
		QDesktopServices::openUrl(m_xRequestedURL);
		return false;
	};
	return QWebPage::acceptNavigationRequest(frame,request,type);
}
