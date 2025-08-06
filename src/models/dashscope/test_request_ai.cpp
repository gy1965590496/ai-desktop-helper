#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QDebug>

void streamAiRequest()
{

    QNetworkAccessManager* manager = new QNetworkAccessManager();

    QUrl url("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString apiKey = "sk-xxxxxxx"; // 请替换成你的
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    // 构造请求体
    QJsonObject json;
    json["model"] = "qwen-plus";
    json["stream"] = true;

    QJsonObject streamOptions;
    streamOptions["include_usage"] = true;
    json["stream_options"] = streamOptions;

    QJsonArray messages;
    QJsonObject systemMsg{{"role", "system"}, {"content", "You are a helpful assistant."}};
    QJsonObject userMsg{{"role", "user"}, {"content", "你是谁？"}};
    messages.append(systemMsg);
    messages.append(userMsg);

    json["messages"] = messages;

    QJsonDocument doc(json);
    QByteArray postData = doc.toJson();

    QNetworkReply* reply = manager->post(request, postData);

    // 缓存未处理数据
    QByteArray buffer;

    QObject::connect(reply, &QIODevice::readyRead, [=]() mutable {
        buffer.append(reply->readAll());

        while (true) {
            int index = buffer.indexOf("\n");

            if (index == -1)
                break;

            QByteArray line = buffer.left(index).trimmed();
            buffer.remove(0, index + 1);

            if (line.startsWith("data: ")) {
                QByteArray jsonData = line.mid(6);

                if (jsonData == "[DONE]") {
                    qDebug() << "\n[流式传输完成]";
                    return;
                }

                QJsonParseError err;
                QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);
                if (err.error != QJsonParseError::NoError) {
                    qWarning() << "解析失败:" << err.errorString();
                    continue;
                }

                QJsonObject obj = doc.object();
                QJsonObject delta = obj["choices"].toArray()[0].toObject()["delta"].toObject();

                QString content = delta["content"].toString();
                if (!content.isEmpty()) {
                    // 实时输出模型内容
                    QTextStream(stdout) << content << flush;
                }
            }
        }
    });

    QObject::connect(reply, &QNetworkReply::finished, []() {
        qDebug() << "\n[连接关闭]";
        QCoreApplication::quit();
    });
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    streamAiRequest();
    return a.exec();
}
