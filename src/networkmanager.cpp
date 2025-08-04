#include "networkmanager.h"
#include <QDebug>
#include <QEventLoop>
#include <QUrlQuery>
#include <QJsonParseError>
/*
    // 测试网络请求
    qDebug() << "测试网络请求...";
    
    // 同步GET请求测试
    NetworkResponse response = networkManager->get("https://httpbin.org/get", {{"test", "value"}});
    if (response.success) {
        qDebug() << "GET请求成功，状态码:" << response.statusCode;
        qDebug() << "响应数据:" << response.data;
    } else {
        qDebug() << "GET请求失败:" << response.errorString;
    }
    
    // 异步POST请求测试
    QJsonObject jsonData;
    jsonData["name"] = "test";
    jsonData["value"] = 123;
    
    networkManager->postJsonAsync("https://httpbin.org/post", jsonData, 
        [](const NetworkResponse &response) {
            if (response.success) {
                qDebug() << "异步POST请求成功，状态码:" << response.statusCode;
                qDebug() << "响应数据:" << response.data;
            } else {
                qDebug() << "异步POST请求失败:" << response.errorString;
            }
        });
*/
NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_manager(nullptr)
{
    initializeManager();
}

NetworkManager::~NetworkManager()
{
    if (m_manager) {
        delete m_manager;
    }
}

void NetworkManager::initializeManager()
{
    m_manager = new QNetworkAccessManager(this);
    
    // 连接信号槽
    connect(m_manager, &QNetworkAccessManager::finished, this, &NetworkManager::onReplyFinished);
}

void NetworkManager::setConfig(const NetworkConfig &config)
{
    m_config = config;
}

NetworkConfig NetworkManager::getConfig() const
{
    return m_config;
}

void NetworkManager::addHeader(const QString &key, const QString &value)
{
    m_config.headers[key] = value;
}

void NetworkManager::removeHeader(const QString &key)
{
    m_config.headers.remove(key);
}

void NetworkManager::clearHeaders()
{
    m_config.headers.clear();
}

// 同步请求方法
NetworkResponse NetworkManager::get(const QString &url, const QMap<QString, QString> &params)
{
    QString fullUrl = buildUrl(url, params);
    return request(RequestType::GET, fullUrl, QByteArray(), "application/json");
}

NetworkResponse NetworkManager::post(const QString &url, const QByteArray &data, const QString &contentType)
{
    return request(RequestType::POST, url, data, contentType);
}

NetworkResponse NetworkManager::postJson(const QString &url, const QJsonObject &jsonData)
{
    QByteArray data = jsonToByteArray(jsonData);
    return post(url, data, "application/json");
}

NetworkResponse NetworkManager::put(const QString &url, const QByteArray &data, const QString &contentType)
{
    return request(RequestType::PUT, url, data, contentType);
}

NetworkResponse NetworkManager::putJson(const QString &url, const QJsonObject &jsonData)
{
    QByteArray data = jsonToByteArray(jsonData);
    return put(url, data, "application/json");
}

NetworkResponse NetworkManager::deleteRequest(const QString &url)
{
    return request(RequestType::DELETE_REQUEST, url, QByteArray(), "application/json");
}

NetworkResponse NetworkManager::request(RequestType type, const QString &url, const QByteArray &data, const QString &contentType)
{
    QNetworkRequest request = createRequest(url);
    
    // 设置内容类型
    if (!contentType.isEmpty()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    }
    
    QNetworkReply *reply = nullptr;
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    // 根据请求类型发送请求
    switch (type) {
        case RequestType::GET:
            reply = m_manager->get(request);
            break;
        case RequestType::POST:
            reply = m_manager->post(request, data);
            break;
        case RequestType::PUT:
            reply = m_manager->put(request, data);
            break;
        case RequestType::DELETE_REQUEST:
            reply = m_manager->deleteResource(request);
            break;
        case RequestType::PATCH:
            // Qt5没有直接的PATCH方法，使用自定义方法
            request.setAttribute(QNetworkRequest::CustomVerbAttribute, "PATCH");
            reply = m_manager->sendCustomRequest(request, "PATCH", data);
            break;
    }
    
    if (!reply) {
        NetworkResponse response;
        response.success = false;
        response.errorString = "Failed to create network reply";
        return response;
    }
    
    // 设置超时定时器
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    m_timeoutTimers[reply] = timer;
    m_requestStartTimes[reply] = startTime;
    
    connect(timer, &QTimer::timeout, this, &NetworkManager::onReplyTimeout);
    timer->start(m_config.timeout);
    
    // 等待请求完成
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), &loop, &QEventLoop::quit);
    
    loop.exec();
    
    // 创建响应
    NetworkResponse response = createResponse(reply, startTime);
    
    // 清理资源
    timer->stop();
    timer->deleteLater();
    m_timeoutTimers.remove(reply);
    m_requestStartTimes.remove(reply);
    reply->deleteLater();
    
    return response;
}

// 异步请求方法（简化版）
void NetworkManager::getAsync(const QString &url, const QMap<QString, QString> &params, std::function<void(const NetworkResponse&)> callback)
{
    QString fullUrl = buildUrl(url, params);
    requestAsync(RequestType::GET, fullUrl, QByteArray(), "application/json", callback);
}

void NetworkManager::postAsync(const QString &url, const QByteArray &data, const QString &contentType, std::function<void(const NetworkResponse&)> callback)
{
    requestAsync(RequestType::POST, url, data, contentType, callback);
}

void NetworkManager::postJsonAsync(const QString &url, const QJsonObject &jsonData, std::function<void(const NetworkResponse&)> callback)
{
    QByteArray data = jsonToByteArray(jsonData);
    postAsync(url, data, "application/json", callback);
}

void NetworkManager::putAsync(const QString &url, const QByteArray &data, const QString &contentType, std::function<void(const NetworkResponse&)> callback)
{
    requestAsync(RequestType::PUT, url, data, contentType, callback);
}

void NetworkManager::putJsonAsync(const QString &url, const QJsonObject &jsonData, std::function<void(const NetworkResponse&)> callback)
{
    QByteArray data = jsonToByteArray(jsonData);
    putAsync(url, data, "application/json", callback);
}

void NetworkManager::deleteAsync(const QString &url, std::function<void(const NetworkResponse&)> callback)
{
    requestAsync(RequestType::DELETE_REQUEST, url, QByteArray(), "application/json", callback);
}

void NetworkManager::requestAsync(RequestType type, const QString &url, const QByteArray &data, const QString &contentType, std::function<void(const NetworkResponse&)> callback)
{
    QNetworkRequest request = createRequest(url);
    
    if (!contentType.isEmpty()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    }
    
    QNetworkReply *reply = nullptr;
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    switch (type) {
        case RequestType::GET:
            reply = m_manager->get(request);
            break;
        case RequestType::POST:
            reply = m_manager->post(request, data);
            break;
        case RequestType::PUT:
            reply = m_manager->put(request, data);
            break;
        case RequestType::DELETE_REQUEST:
            reply = m_manager->deleteResource(request);
            break;
        case RequestType::PATCH:
            request.setAttribute(QNetworkRequest::CustomVerbAttribute, "PATCH");
            reply = m_manager->sendCustomRequest(request, "PATCH", data);
            break;
    }
    
    if (!reply) {
        NetworkResponse response;
        response.success = false;
        response.errorString = "Failed to create network reply";
        if (callback) callback(response);
        emit requestFinished(response);
        emit requestError(response.errorString);
        return;
    }
    
    // 设置超时定时器
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    m_timeoutTimers[reply] = timer;
    m_requestStartTimes[reply] = startTime;
    m_callbacks[reply] = callback;
    
    connect(timer, &QTimer::timeout, this, &NetworkManager::onReplyTimeout);
    timer->start(m_config.timeout);
    
    // 连接信号
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &NetworkManager::onReplyError);
    connect(reply, &QNetworkReply::downloadProgress, this, &NetworkManager::onReplyProgress);
}

// 工具方法
QString NetworkManager::buildUrl(const QString &url, const QMap<QString, QString> &params) const
{
    if (params.isEmpty()) {
        return url;
    }
    
    QUrl qurl(url);
    QUrlQuery query;
    
    for (auto it = params.begin(); it != params.end(); ++it) {
        query.addQueryItem(it.key(), it.value());
    }
    
    qurl.setQuery(query);
    return qurl.toString();
}

QByteArray NetworkManager::jsonToByteArray(const QJsonObject &json) const
{
    QJsonDocument doc(json);
    return doc.toJson();
}

QJsonObject NetworkManager::byteArrayToJson(const QByteArray &data) const
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << error.errorString();
        return QJsonObject();
    }
    
    return doc.object();
}

bool NetworkManager::isValidJson(const QByteArray &data) const
{
    QJsonParseError error;
    QJsonDocument::fromJson(data, &error);
    return error.error == QJsonParseError::NoError;
}

// 私有方法
QNetworkRequest NetworkManager::createRequest(const QString &url) const
{
    QString fullUrl = url;
    if (!m_config.baseUrl.isEmpty() && !url.startsWith("http")) {
        fullUrl = m_config.baseUrl + url;
    }
    
    QNetworkRequest request;
    request.setUrl(QUrl(fullUrl));
    
    // 设置请求头
    for (auto it = m_config.headers.begin(); it != m_config.headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    
    // 设置重定向策略
    if (m_config.followRedirects) {
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    }
    
    return request;
}

NetworkResponse NetworkManager::createResponse(QNetworkReply *reply, qint64 startTime) const
{
    NetworkResponse response;
    
    if (!reply) {
        response.success = false;
        response.errorString = "Invalid reply";
        return response;
    }
    
    response.success = (reply->error() == QNetworkReply::NoError);
    response.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    response.data = reply->readAll();
    response.errorString = reply->errorString();
    response.responseTime = QDateTime::currentMSecsSinceEpoch() - startTime;
    
    // 获取响应头
    QList<QByteArray> headerNames = reply->rawHeaderList();
    for (const QByteArray &name : headerNames) {
        response.headers[QString::fromUtf8(name)] = QString::fromUtf8(reply->rawHeader(name));
    }
    
    return response;
}

void NetworkManager::handleReply(QNetworkReply *reply)
{
    if (!reply) return;
    
    qint64 startTime = m_requestStartTimes.value(reply, 0);
    NetworkResponse response = createResponse(reply, startTime);
    
    // 清理资源
    QTimer *timer = m_timeoutTimers.value(reply);
    if (timer) {
        timer->stop();
        timer->deleteLater();
        m_timeoutTimers.remove(reply);
    }
    
    m_requestStartTimes.remove(reply);
    std::function<void(const NetworkResponse&)> callbackFunc = m_callbacks.take(reply);
    reply->deleteLater();
    
    if (callbackFunc) {
        callbackFunc(response);
    }
    
    // 发送信号
    emit requestFinished(response);
    
    if (!response.success) {
        emit requestError(response.errorString);
    }
}

void NetworkManager::retryRequest(QNetworkReply *reply, int retryCount)
{
    if (retryCount >= m_config.maxRetries) {
        handleReply(reply);
        return;
    }
    
    // 延迟重试
    QTimer::singleShot(m_config.retryDelay, [this, reply, retryCount]() {
        handleReply(reply);
    });
}

// 槽函数
void NetworkManager::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        handleReply(reply);
    }
}

void NetworkManager::onReplyError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        qDebug() << "Network error:" << error << reply->errorString();
        handleReply(reply);
    }
}

void NetworkManager::onReplyProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit requestProgress(bytesReceived, bytesTotal);
}

void NetworkManager::onReplyTimeout()
{
    QTimer *timer = qobject_cast<QTimer*>(sender());
    if (timer) {
        QNetworkReply *reply = m_timeoutTimers.key(timer);
        if (reply) {
            reply->abort();
            handleReply(reply);
        }
    }
} 