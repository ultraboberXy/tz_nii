#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

class DataFetcher : public QObject {
    Q_OBJECT
public:
    explicit DataFetcher(QObject *parent = nullptr);
    void fetchUrl(const QUrl &url);
    void abort();

signals:
    void finished(const QByteArray &data, const QUrl &finalUrl);
    void errorOccurred(const QString &message);
    void progress(int percent);

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onDownloadProgress(qint64 received, qint64 total);

private:
    QNetworkAccessManager *m_nam;
    QNetworkReply *m_currentReply = nullptr;
    static QByteArray detectEncoding(const QByteArray &html);
};