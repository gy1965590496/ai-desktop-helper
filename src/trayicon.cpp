#include "trayicon.h"
#include "floatingball.h"
#include <QApplication>
#include <QPainter>
#include <QFont>

TrayIcon::TrayIcon(FloatingBall *ball, QObject *parent)
    : QObject(parent)
    , m_ball(ball)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_showAction(nullptr)
    , m_hideAction(nullptr)
    , m_exitAction(nullptr)
{
    createTrayIcon();
    createTrayMenu();
}

void TrayIcon::show()
{
    if (m_trayIcon) {
        m_trayIcon->show();
    }
}

void TrayIcon::hide()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

bool TrayIcon::isVisible() const
{
    return m_trayIcon ? m_trayIcon->isVisible() : false;
}

void TrayIcon::setIcon(const QPixmap &pixmap)
{
    if (m_trayIcon) {
        m_trayIcon->setIcon(QIcon(pixmap));
    }
}

void TrayIcon::setToolTip(const QString &tooltip)
{
    if (m_trayIcon) {
        m_trayIcon->setToolTip(tooltip);
    }
}

void TrayIcon::addMenuAction(QAction *action)
{
    if (m_trayMenu && action) {
        m_trayMenu->addAction(action);
    }
}

void TrayIcon::removeMenuAction(QAction *action)
{
    if (m_trayMenu && action) {
        m_trayMenu->removeAction(action);
    }
}

void TrayIcon::clearMenuActions()
{
    if (m_trayMenu) {
        m_trayMenu->clear();
    }
}

void TrayIcon::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        emit trayIconDoubleClicked();
        
        if (m_ball) {
            if (m_ball->isBallVisible()) {
                m_ball->hideBall();
            } else {
                m_ball->showBall();
                m_ball->moveToTrayArea();
            }
        }
    }
}

void TrayIcon::onShowBallTriggered()
{
    if (m_ball) {
        m_ball->showBall();
        m_ball->moveToTrayArea();
    }
}

void TrayIcon::onHideBallTriggered()
{
    if (m_ball) {
        m_ball->hideBall();
    }
}

void TrayIcon::onExitTriggered()
{
    emit exitRequested();
}

void TrayIcon::createTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    
    // 设置默认图标
    QPixmap iconPixmap = createDefaultIcon();
    m_trayIcon->setIcon(QIcon(iconPixmap));
    m_trayIcon->setToolTip("悬浮球应用");
    
    // 连接激活信号
    connect(m_trayIcon, &QSystemTrayIcon::activated, 
            this, &TrayIcon::onTrayIconActivated);
}

void TrayIcon::createTrayMenu()
{
    m_trayMenu = new QMenu();
    
    // 创建菜单项
    m_showAction = new QAction("显示悬浮球", m_trayMenu);
    m_hideAction = new QAction("隐藏悬浮球", m_trayMenu);
    m_exitAction = new QAction("退出", m_trayMenu);
    
    // 添加菜单项
    m_trayMenu->addAction(m_showAction);
    m_trayMenu->addAction(m_hideAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_exitAction);
    
    // 设置托盘菜单
    m_trayIcon->setContextMenu(m_trayMenu);
    
    // 连接信号槽
    connect(m_showAction, &QAction::triggered, this, &TrayIcon::onShowBallTriggered);
    connect(m_hideAction, &QAction::triggered, this, &TrayIcon::onHideBallTriggered);
    connect(m_exitAction, &QAction::triggered, this, &TrayIcon::onExitTriggered);
}

QPixmap TrayIcon::createDefaultIcon()
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制蓝色圆形，带白色边框
    painter.setBrush(QColor(0, 100, 255));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(4, 4, 24, 24);
    
    // 绘制白色文字 "K"
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "K");
    
    return pixmap;
} 