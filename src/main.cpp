#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include "floatingball.h"
#include "trayicon.h"

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
    
    // 连接信号槽
    QObject::connect(trayIcon, &TrayIcon::exitRequested, [&app]() {
        app.quit();
    });
    
    // 连接悬浮球菜单项点击信号（预留扩展点）
    QObject::connect(ball, &FloatingBall::menuItemClicked, [](const QString &itemText) {
        // 预留：这里可以添加全局的菜单项处理逻辑
        qDebug() << "菜单项被点击:" << itemText;
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
    
    // 运行应用程序
    return app.exec();
} 