#include <QDateTime>
#include <QThread>
#include <QtGlobal>

/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "cookiejar.h"
#include "downloadmanager.h"
#include "networkaccessmanager.h"
#include "tabwidget.h"
#include "webview.h"

#include <QtGui/QClipboard>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>

#include <QtWebKit/QWebHitTestResult>

#ifndef QT_NO_UITOOLS
#include <QtUiTools/QUiLoader>
#endif  //QT_NO_UITOOLS

#include <QtCore/QDebug>
#include <QtCore/QBuffer>


QFile inputFile(QLatin1String("/home/sslab/log.trace"));

WebPage::WebPage(QObject *parent)
    : QWebPage(parent)
    , m_keyboardModifiers(Qt::NoModifier)
    , m_pressedButtons(Qt::NoButton)
    , m_openInNewTab(false)
{
    setNetworkAccessManager(BrowserApplication::networkAccessManager());
    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)),
            this, SLOT(handleUnsupportedContent(QNetworkReply*)));
}

BrowserMainWindow *WebPage::mainWindow()
{
    QObject *w = this->parent();
    while (w) {
        if (BrowserMainWindow *mw = qobject_cast<BrowserMainWindow*>(w))
            return mw;
        w = w->parent();
    }
    return BrowserApplication::instance()->mainWindow();
}

bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    // ctrl open in new tab
    // ctrl-shift open in new tab and select
    // ctrl-alt open in new window
    if (type == QWebPage::NavigationTypeLinkClicked
        && (m_keyboardModifiers & Qt::ControlModifier
            || m_pressedButtons == Qt::MidButton)) {
        bool newWindow = (m_keyboardModifiers & Qt::AltModifier);
        WebView *webView;
        if (newWindow) {
            BrowserApplication::instance()->newMainWindow();
            BrowserMainWindow *newMainWindow = BrowserApplication::instance()->mainWindow();
            webView = newMainWindow->currentTab();
            newMainWindow->raise();
            newMainWindow->activateWindow();
            webView->setFocus();
        } else {
            bool selectNewTab = (m_keyboardModifiers & Qt::ShiftModifier);
            webView = mainWindow()->tabWidget()->newTab(selectNewTab);
        }
        webView->load(request);
        m_keyboardModifiers = Qt::NoModifier;
        m_pressedButtons = Qt::NoButton;
        return false;
    }
    if (frame == mainFrame()) {
        m_loadingUrl = request.url();
        emit loadingUrl(m_loadingUrl);
    }
    return QWebPage::acceptNavigationRequest(frame, request, type);
}

QWebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
    Q_UNUSED(type);
    if (m_keyboardModifiers & Qt::ControlModifier || m_pressedButtons == Qt::MidButton)
        m_openInNewTab = true;
    if (m_openInNewTab) {
        m_openInNewTab = false;
        return mainWindow()->tabWidget()->newTab()->page();
    }
    BrowserApplication::instance()->newMainWindow();
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();
    return mainWindow->currentTab()->page();
}

#if !defined(QT_NO_UITOOLS)
QObject *WebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    Q_UNUSED(url);
    Q_UNUSED(paramNames);
    Q_UNUSED(paramValues);
    QUiLoader loader;
    return loader.createWidget(classId, view());
}
#endif // !defined(QT_NO_UITOOLS)

void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    QString errorString = reply->errorString();

    if (m_loadingUrl != reply->url()) {
        // sub resource of this page
        qWarning() << "Resource" << reply->url().toEncoded() << "has unknown Content-Type, will be ignored.";
        reply->deleteLater();
        return;
    }

    if (reply->error() == QNetworkReply::NoError && !reply->header(QNetworkRequest::ContentTypeHeader).isValid()) {
        errorString = "Unknown Content-Type";
    }

    QFile file(QLatin1String(":/notfound.html"));
    bool isOpened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(isOpened);
    Q_UNUSED(isOpened)

    QString title = tr("Error loading page: %1").arg(reply->url().toString());
    QString html = QString(QLatin1String(file.readAll()))
                        .arg(title)
                        .arg(errorString)
                        .arg(reply->url().toString());

    QBuffer imageBuffer;
    imageBuffer.open(QBuffer::ReadWrite);
    QIcon icon = view()->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, view());
    QPixmap pixmap = icon.pixmap(QSize(32,32));
    if (pixmap.save(&imageBuffer, "PNG")) {
        html.replace(QLatin1String("IMAGE_BINARY_DATA_HERE"),
                     QString(QLatin1String(imageBuffer.buffer().toBase64())));
    }

    QList<QWebFrame*> frames;
    frames.append(mainFrame());
    while (!frames.isEmpty()) {
        QWebFrame *frame = frames.takeFirst();
        if (frame->url() == reply->url()) {
            frame->setHtml(html, reply->url());
            return;
        }
        QList<QWebFrame *> children = frame->childFrames();
        foreach(QWebFrame *frame, children)
            frames.append(frame);
    }
    if (m_loadingUrl == reply->url()) {
        mainFrame()->setHtml(html, reply->url());
    }
}

WebView::WebView(QWidget* parent)
    : QWebView(parent)
    , m_progress(0)
    , m_page(new WebPage(this))

// SSLAB : Real tracing is basically disabled.
    , tflag(false)
{
    setPage(m_page);
    connect(page(), SIGNAL(statusBarMessage(QString)),
            SLOT(setStatusBarText(QString)));
    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished()));
    connect(page(), SIGNAL(loadingUrl(QUrl)),
            this, SIGNAL(urlChanged(QUrl)));
    connect(page(), SIGNAL(downloadRequested(QNetworkRequest)),
            this, SLOT(downloadRequested(QNetworkRequest)));
    page()->setForwardUnsupportedContent(true);

// SSLAB : Initialization for measuring loading time.
    set_totalPageLoadingTime(0);
    set_cntPage(0);
	
    QFile outputDetailFile(QLatin1String("/home/sslab/log.page.detail.csv"));
    if(!outputDetailFile.open(QFile::WriteOnly)) {
        printf("[tracing] outputFile open error in %s\n", __func__);
	exit(-1);
    }

    char buff[4096] = {'\0', };
    sprintf(buff, "\"Time(msec)\",\"URL\"\n");
    outputDetailFile.write(buff, strlen(buff));
    outputDetailFile.close();
// ~SSLAB
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());
    if (!r.linkUrl().isEmpty()) {
        QMenu menu(this);
        menu.addAction(pageAction(QWebPage::OpenLinkInNewWindow));
        menu.addAction(tr("Open in New Tab"), this, SLOT(openLinkInNewTab()));
        menu.addSeparator();
        menu.addAction(pageAction(QWebPage::DownloadLinkToDisk));
        // Add link to bookmarks...
        menu.addSeparator();
        menu.addAction(pageAction(QWebPage::CopyLinkToClipboard));
        if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled))
            menu.addAction(pageAction(QWebPage::InspectElement));
        menu.exec(mapToGlobal(event->pos()));
        return;
    }
    QWebView::contextMenuEvent(event);
}

void WebView::wheelEvent(QWheelEvent *event)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        setTextSizeMultiplier(textSizeMultiplier() + numSteps * 0.1);
        event->accept();
        return;
    }
    QWebView::wheelEvent(event);
}

void WebView::openLinkInNewTab()
{
    m_page->m_openInNewTab = true;
    pageAction(QWebPage::OpenLinkInNewWindow)->trigger();
}

void WebView::setProgress(int progress)
{
    m_progress = progress;
}

void WebView::loadFinished()
{
    if (100 != m_progress) {
        qWarning() << "Received finished signal while progress is still:" << progress()
                   << "Url:" << url();
    }
    m_progress = 0;

	QDateTime currentTime = QDateTime::currentDateTime();
	qint64 duration = currentTime.currentMSecsSinceEpoch() - inittime;
	printf("%lld - after Step 1\n", duration);

// SSLAB : Timestamp. endpoint of the page loading.
    currentTime = QDateTime::currentDateTime();
    qint64 endTime = currentTime.currentMSecsSinceEpoch();
    PageLoadTime = endTime - currentPageLoadStartTime;
    qWarning("SSLAB : Finish loading : %lld ms, %s \n",
        PageLoadTime, url().toString().toAscii().data());

// SSLAB : Logging average page loading time of real trace.
    if(tflag) {    
        qint64 finishTime = QDateTime::currentDateTime().currentMSecsSinceEpoch();
        QFile outputFile(QLatin1String("/home/sslab/log.page"));
        if(!outputFile.open(QFile::WriteOnly | QFile::Append)) {
            printf("[tracing] outputFile open error in %s\n", __func__);
            exit(-1);
        }
    
        inc_cntPage();
        qint64 elapsedTime = finishTime-pageLoadingStartTime;
        add_totalPageLoadingTime(elapsedTime);

        char buff[4096] = {0, };
        sprintf(buff, "Avg. Page Loading Time: %f msec.(cnt: %d)\n",
            (double)get_totalPageLoadingTime()/(double)get_cntPage(), get_cntPage());
        outputFile.write(buff, strlen(buff));
        outputFile.close();
	
        QFile outputDetailFile(QLatin1String("/home/sslab/log.page.detail.csv"));
        if(!outputDetailFile.open(QFile::WriteOnly | QFile::Append)) {
            printf("[tracing] outputFile open error in %s\n", __func__);
            exit(-1);
        }

        sprintf(buff, "\"%f\",\"%s\"\n", (double)elapsedTime, url().toString().toAscii().data());
        outputDetailFile.write(buff, strlen(buff));
        outputDetailFile.close();
	
        printf("[tracing] we will request next webpage. (%s)\n", url().toString().toAscii().data());
        char newURL[4096] = {0, };
        if(inputFile.readLine(newURL, 4096) > 0 && strlen(newURL) > 1)
            loadUrl(QUrl::fromUserInput(QLatin1String(newURL)));
        else {
            printf("[tracing] finish to trace.\n");
            inputFile.close();
        }
    }
// ~SSLAB
}

// SSLAB
void WebView::boolLoadUrl()
{
    boolLoad();
}

// SSLAB
void WebView::setLoadable()
{
    loadable();
}

// SSLAB
void WebView::setDebugging()
{
    debugging();
}

// SSLAB
void WebView::setPCaching()
{
    pCaching();
}

// SSLAB
void WebView::setTracing()
{
    if(tflag) { printf("SSLAB : Disabled tracing.\n"); tflag = false; } 
    else { printf("SSLAB : Enabled tracing.\n"); tflag = true; }
    tracing();
}

void WebView::loadUrl(const QUrl &url)
{

    boolLoadUrl();

// SSLAB : Stop real tracing when the url is equal to "about:blank".
    if(tflag) {
        if(!strcmp(url.toString().toAscii().data(), "about:blank")) {
            if(!inputFile.open(QFile::ReadOnly)) {
                printf("[tracing] openFile open error!\n");
                exit(-1);
            }
        }
    }
// ~SSLAB

// SSLAB : Timestamp. startpoint of the page loading.
    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 startTime = currentTime.currentMSecsSinceEpoch();
    currentPageLoadStartTime = startTime;
    qWarning("SSLAB : Start loading : %s (%lld ms)\n",
        url.toString().toAscii().data(), startTime);
    
    pageLoadingStartTime = QDateTime::currentDateTime().currentMSecsSinceEpoch();
// ~SSLAB

    m_initialUrl = url;
    load(url);
}

QString WebView::lastStatusBarText() const
{
    return m_statusBarText;
}

QUrl WebView::url() const
{
    QUrl url = QWebView::url();
    if (!url.isEmpty())
        return url;

    return m_initialUrl;
}

void WebView::mousePressEvent(QMouseEvent *event)
{
    m_page->m_pressedButtons = event->buttons();
    m_page->m_keyboardModifiers = event->modifiers();
    QWebView::mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    QWebView::mouseReleaseEvent(event);
    if (!event->isAccepted() && (m_page->m_pressedButtons & Qt::MidButton)) {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) {
            setUrl(url);
        }
    }
}

void WebView::setStatusBarText(const QString &string)
{
    m_statusBarText = string;
}

void WebView::downloadRequested(const QNetworkRequest &request)
{
    BrowserApplication::downloadManager()->download(request);
}

// SSLAB : for real tracing.
void WebView::set_cntPage(unsigned int t_cntPage) {
	this->t_cntPage = t_cntPage;
}

// SSLAB : for real tracing.
unsigned int WebView::get_cntPage() {
	return this->t_cntPage;
}

// SSLAB : for real tracing.
void WebView::inc_cntPage() {
	set_cntPage(get_cntPage()+1);
}

// SSLAB : for real tracing.
void WebView::set_totalPageLoadingTime(qint64 t_totalPageLoadingTime) {
	this->t_totalPageLoadingTime = t_totalPageLoadingTime;
}

// SSLAB : for real tracing.
qint64 WebView::get_totalPageLoadingTime() {
	return this->t_totalPageLoadingTime;
}

// SSLAB : for real tracing.
void WebView::add_totalPageLoadingTime(qint64 additional) {
	set_totalPageLoadingTime(get_totalPageLoadingTime() + additional);
}

