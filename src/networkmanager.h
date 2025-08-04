#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QMap>
#include <QTimer>
#include <functional> // Added for std::function

// 网络请求配置结构体
struct NetworkConfig {
    QString baseUrl;
    int timeout;
    QMap<QString, QString> headers;
    bool followRedirects;
    int maxRetries;
    int retryDelay;
    
    NetworkConfig() : timeout(30000), followRedirects(true), maxRetries(3), retryDelay(1000) {}
};

// 网络请求结果结构体
struct NetworkResponse {
    bool success;
    int statusCode;
    QByteArray data;
    QString errorString;
    QMap<QString, QString> headers;
    qint64 responseTime;
};

// 网络请求类型枚举
enum class RequestType {
    GET,
    POST,
    PUT,
    DELETE_REQUEST,
    PATCH
};

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    // 配置方法
    void setConfig(const NetworkConfig &config);
    NetworkConfig getConfig() const;
    
    // 添加/移除请求头
    void addHeader(const QString &key, const QString &value);
    void removeHeader(const QString &key);
    void clearHeaders();

    // 同步请求方法
    NetworkResponse get(const QString &url, const QMap<QString, QString> &params);
    NetworkResponse post(const QString &url, const QByteArray &data, const QString &contentType);
    NetworkResponse postJson(const QString &url, const QJsonObject &jsonData);
    NetworkResponse put(const QString &url, const QByteArray &data, const QString &contentType);
    NetworkResponse putJson(const QString &url, const QJsonObject &jsonData);
    NetworkResponse deleteRequest(const QString &url);
    NetworkResponse request(RequestType type, const QString &url, const QByteArray &data, const QString &contentType);

    // 异步请求方法
    void getAsync(const QString &url, const QMap<QString, QString> &params, std::function<void(const NetworkResponse&)> callback = nullptr);
    void postAsync(const QString &url, const QByteArray &data, const QString &contentType, std::function<void(const NetworkResponse&)> callback = nullptr);
    void postJsonAsync(const QString &url, const QJsonObject &jsonData, std::function<void(const NetworkResponse&)> callback = nullptr);
    void putAsync(const QString &url, const QByteArray &data, const QString &contentType, std::function<void(const NetworkResponse&)> callback = nullptr);
    void putJsonAsync(const QString &url, const QJsonObject &jsonData, std::function<void(const NetworkResponse&)> callback = nullptr);
    void deleteAsync(const QString &url, std::function<void(const NetworkResponse&)> callback = nullptr);
    void requestAsync(RequestType type, const QString &url, const QByteArray &data, const QString &contentType, std::function<void(const NetworkResponse&)> callback = nullptr);

    // 工具方法
    QString buildUrl(const QString &url, const QMap<QString, QString> &params) const;
    QByteArray jsonToByteArray(const QJsonObject &json) const;
    QJsonObject byteArrayToJson(const QByteArray &data) const;
    bool isValidJson(const QByteArray &data) const;

signals:
    // 请求完成信号
    void requestFinished(const NetworkResponse &response);
    
    // 请求错误信号
    void requestError(const QString &error);
    
    // 请求进度信号
    void requestProgress(qint64 bytesReceived, qint64 bytesTotal);

private slots:
    // 网络回复槽函数
    void onReplyFinished();
    void onReplyError(QNetworkReply::NetworkError error);
    void onReplyProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onReplyTimeout();

private:
    // 私有方法
    void initializeManager();
    QNetworkRequest createRequest(const QString &url) const;
    NetworkResponse createResponse(QNetworkReply *reply, qint64 startTime) const;
    void handleReply(QNetworkReply *reply);
    void retryRequest(QNetworkReply *reply, int retryCount);
    
    // 成员变量
    QNetworkAccessManager *m_manager;
    NetworkConfig m_config;
    QMap<QNetworkReply*, QTimer*> m_timeoutTimers;
    QMap<QNetworkReply*, qint64> m_requestStartTimes;
    QMap<QNetworkReply*, std::function<void(const NetworkResponse&)>> m_callbacks; // Added for callbacks
};

#endif // NETWORKMANAGER_H 