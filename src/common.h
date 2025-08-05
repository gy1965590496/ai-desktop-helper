#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QDateTime>
#include <QPixmap>
#include <QIcon>

// 应用记录结构体
struct AppRecord {
    QString appName;
    QDateTime timestamp;
    QPixmap screenshot;
    QString appPath;
    QString windowTitle;
};

#endif // COMMON_H 