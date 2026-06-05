#include "datafetcher.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QDebug>
#include <QRegularExpression>

DataFetcher::DataFetcher(QObject *parent) : QObject(parent) {
    m_nam = new QNetworkAccessManager(this);
    connect(m_nam, &QNetworkAccessManager::finished,
            this, &DataFetcher::onReplyFinished);
}

void DataFetcher::fetchUrl(const QUrl &url) {
    qDebug() << "[FETCH] Начинаем загрузку:" << url.toString();

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;
    }

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                  "AppleWebKit/537.36 (KHTML, like Gecko) "
                  "Chrome/124.0.0.0 Safari/537.36");
    req.setRawHeader("Accept",
                     "text/html,application/xhtml+xml,application/json,"
                     "application/xml;q=0.9,*/*;q=0.8");
    req.setRawHeader("Accept-Language", "ru-RU,ru;q=0.9,en;q=0.8");
    req.setRawHeader("Accept-Encoding", "identity");
    req.setRawHeader("Connection", "keep-alive");
    req.setRawHeader("Cache-Control", "no-cache");

    QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
    ssl.setPeerVerifyMode(QSslSocket::VerifyNone);
    req.setSslConfiguration(ssl);

    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    m_currentReply = m_nam->get(req);
    connect(m_currentReply, &QNetworkReply::downloadProgress,
            this, &DataFetcher::onDownloadProgress);
}

void DataFetcher::abort() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;
    }
}

void DataFetcher::onReplyFinished(QNetworkReply *reply) {
    reply->deleteLater();
    m_currentReply = nullptr;

    if (reply->error() != QNetworkReply::NoError &&
        reply->error() != QNetworkReply::OperationCanceledError) {
        qDebug() << "[FETCH] Ошибка сети:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        return;
    }

    int statusCode = reply->attribute(
                              QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "[FETCH] HTTP статус:" << statusCode;

    if (statusCode >= 400) {
        QString errorMsg = QString("HTTP %1: %2").arg(statusCode)
        .arg(reply->attribute(
                      QNetworkRequest::HttpReasonPhraseAttribute).toString());
        qDebug() << "[FETCH] Ошибка HTTP:" << errorMsg;
        emit errorOccurred(errorMsg);
        return;
    }

    QByteArray raw = reply->readAll();
    qDebug() << "[FETCH] Получено байт:" << raw.size();

    if (raw.isEmpty()) {
        qDebug() << "[FETCH] Пустой ответ";
        emit errorOccurred("Пустой ответ от сервера");
        return;
    }

    qDebug() << "[FETCH] Успешно получили данные, передаем в парсер";
    emit finished(raw, reply->url());
}

void DataFetcher::onDownloadProgress(qint64 received, qint64 total) {
    if (total > 0) {
        emit progress(static_cast<int>(received * 100 / total));
    } else {
        emit progress(-1);
    }
}

QByteArray DataFetcher::detectEncoding(const QByteArray &html) {
    static QRegularExpression re(
        R"(charset\s*=\s*[\"']?\s*([\w-]+))",
        QRegularExpression::CaseInsensitiveOption);
    auto m = re.match(QString::fromLatin1(html.left(2048)));
    if (m.hasMatch()) {
        return m.captured(1).toLatin1();
    }
    return "utf-8";
}