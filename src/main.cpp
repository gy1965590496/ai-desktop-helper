#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include "floatingball.h"
#include "trayicon.h"
#include "screenmonitor.h"
#include <QDir> // Added for QDir::currentPath()

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 检查系统托盘是否可用
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "错误", "系统托盘不可用，无法启动应用。");
        return 1;
    }
    
    // 创建悬浮球对象
    FloatingBall *ball = new FloatingBall();
    
    // 创建托盘图标管理器
    TrayIcon *trayIcon = new TrayIcon(ball);
    
    // 创建屏幕监控模块
    ScreenMonitor *screenMonitor = new ScreenMonitor();
    
    // 配置屏幕监控
    ScreenshotConfig config;
    config.captureInterval = 10000; // 10秒截图一次
    config.autoSave = true;
    config.savePath = "./screenshots/";
    config.imageQuality = 85;
    config.maxCacheSize = 50;
    screenMonitor->setConfig(config);
    
    // 连接屏幕监控信号
    QObject::connect(screenMonitor, &ScreenMonitor::activeApplicationChanged, 
        [ball, screenMonitor](const QString &oldApp, const QString &newApp) {
            qDebug() << "应用切换:" << oldApp << "->" << newApp;
        });
    
    QObject::connect(screenMonitor, &ScreenMonitor::appInfoUpdated, 
        [ball](const QString &appName, const AppInfo &appInfo) {
            qDebug() << "应用信息更新完成:" << appName;
            // 设置悬浮球左上角图标
            ball->setAppIcon(appInfo.appIcon);
        });
    
    QObject::connect(screenMonitor, &ScreenMonitor::screenshotCaptured, 
        [](const QString &appName, const QPixmap &screenshot) {
            qDebug() << "截图完成:" << appName << "尺寸:" << screenshot.size();
        });
    
    QObject::connect(screenMonitor, &ScreenMonitor::errorOccurred, 
        [](const QString &error) {
            qDebug() << "屏幕监控错误:" << error;
        });
    
    // 连接信号槽
    QObject::connect(trayIcon, &TrayIcon::exitRequested, [&app]() {
        app.quit();
    });
    
    // 连接悬浮球菜单项点击信号（预留扩展点）
    QObject::connect(ball, &FloatingBall::menuItemClicked, [screenMonitor](const QString &itemText) {
        // 处理菜单项点击
        if (itemText == "屏幕监控") {
            // 启动/停止屏幕监控
            if (screenMonitor->isMonitoring()) {
                screenMonitor->stopMonitoring();
                qDebug() << "屏幕监控已停止";
            } else {
                screenMonitor->startMonitoring();
                qDebug() << "屏幕监控已启动";
            }
        } else {
            qDebug() << "菜单项被点击:" << itemText;
        }
    });
    
    // 连接悬浮球拖拽状态变化信号（预留扩展点）
    QObject::connect(ball, &FloatingBall::dragStateChanged, [](bool isDragging) {
        // 预留：这里可以添加拖拽状态变化的处理逻辑
        qDebug() << "拖拽状态变化:" << (isDragging ? "开始拖拽" : "结束拖拽");
    });
    
    // 连接鼠标进入/离开信号（预留扩展点）
    QObject::connect(ball, &FloatingBall::mouseEntered, []() {
        // 预留：这里可以添加鼠标进入的处理逻辑
        qDebug() << "鼠标进入悬浮球";
    });
    
    QObject::connect(ball, &FloatingBall::mouseLeft, []() {
        // 预留：这里可以添加鼠标离开的处理逻辑
        qDebug() << "鼠标离开悬浮球";
    });
    
    // 移动悬浮球到托盘区域
    ball->moveToTrayArea();
    
    // 显示托盘图标
    trayIcon->show();
    
    // 启动屏幕监控（可选，默认不启动）
    // screenMonitor->startMonitoring();
    
    // 运行应用程序
    return app.exec();
} 