#ifndef TRAYICON_H
#define TRAYICON_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QPixmap>

// 前向声明
class FloatingBall;

// 托盘图标管理类
class TrayIcon : public QObject
{
    Q_OBJECT

public:
    // 构造函数
    explicit TrayIcon(FloatingBall *ball, QObject *parent = nullptr);
    
    // 显示/隐藏托盘图标
    void show();
    void hide();
    bool isVisible() const;
    
    // 设置托盘图标
    void setIcon(const QPixmap &pixmap);
    void setToolTip(const QString &tooltip);
    
    // 托盘菜单管理
    void addMenuAction(QAction *action);
    void removeMenuAction(QAction *action);
    void clearMenuActions();

private slots:
    // 托盘图标激活事件
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    
    // 菜单项响应
    void onShowBallTriggered();
    void onHideBallTriggered();
    void onExitTriggered();

signals:
    // 托盘图标双击信号
    void trayIconDoubleClicked();
    
    // 退出应用信号
    void exitRequested();

private:
    // 私有方法
    void createTrayIcon();
    void createTrayMenu();
    QPixmap createDefaultIcon();
    
    // 成员变量
    QSystemTrayIcon *m_trayIcon;      // 系统托盘图标
    QMenu *m_trayMenu;                // 托盘菜单
    QAction *m_showAction;            // 显示悬浮球动作
    QAction *m_hideAction;            // 隐藏悬浮球动作
    QAction *m_exitAction;            // 退出动作
    FloatingBall *m_ball;             // 悬浮球对象
};

#endif // TRAYICON_H 