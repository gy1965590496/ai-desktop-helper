#include "floatingball.h"
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDebug> // Added for qDebug

// 构造函数
FloatingBall::FloatingBall(QWidget *parent)
    : QWidget(parent)
    , m_isDragging(false)
    , m_contextMenu(nullptr)
    , m_currentTheme(ThemeType::Default)
    , m_movie(nullptr)
    , m_menuVisible(false)
{
    initializeUI();
    setupTimer();
    moveToScreenCenter();
    show();
}

void FloatingBall::initializeUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(m_appearance.size, m_appearance.size);
    setMouseTracking(true);
}

void FloatingBall::setupTimer()
{
    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    m_showTimer->setInterval(500);
    connect(m_showTimer, &QTimer::timeout, this, &FloatingBall::onShowTimerTimeout);
}

void FloatingBall::setAppearance(const FloatingBallAppearance &appearance)
{
    m_appearance = appearance;
    setFixedSize(m_appearance.size, m_appearance.size);
    update();
}

// 获取外观配置
FloatingBallAppearance FloatingBall::getAppearance() const
{
    return m_appearance;
}

// 设置主题
void FloatingBall::setTheme(ThemeType theme)
{
    m_currentTheme = theme;
    
    switch (theme) {
        case ThemeType::Default:
            // 默认主题：蓝色圆形
            m_appearance.backgroundColor = QColor(0, 100, 255);
            m_appearance.borderColor = QColor(255, 255, 255);
            m_appearance.textColor = QColor(255, 255, 255);
            m_appearance.dragColor = QColor(0, 80, 200);
            m_appearance.text = "K";
            m_appearance.useImage = false;
            break;
            
        case ThemeType::JinXiaoMeng:
            // 金小獴主题：使用GIF动画
            m_appearance.useImage = true;
            m_appearance.imagePath = ":/images/images/themes/meng.gif";
            m_appearance.text = "";
            break;
    }
    
    // 更新动画
    updateMovie();
    
    // 更新外观
    setFixedSize(m_appearance.size, m_appearance.size);
    update();
}

// 获取当前主题
ThemeType FloatingBall::getCurrentTheme() const
{
    return m_currentTheme;
}

void FloatingBall::moveToTrayArea()
{
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int trayX = screenGeometry.right() - width() - 20;
    int trayY = screenGeometry.bottom() - height() - 20;
    move(trayX, trayY);
}

void FloatingBall::moveToScreenCenter()
{
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int centerX = screenGeometry.center().x() - width() / 2;
    int centerY = screenGeometry.center().y() - height() / 2;
    move(centerX, centerY);
}

void FloatingBall::setMenuItems(const QList<MenuItemConfig> &items)
{
    m_menuItems = items;
    createMenuItems();
}

void FloatingBall::addMenuItem(const MenuItemConfig &item)
{
    m_menuItems.append(item);
    createMenuItems();
}

void FloatingBall::clearMenuItems()
{
    m_menuItems.clear();
    if (m_contextMenu) {
        m_contextMenu->clear();
    }
}

void FloatingBall::showBall()
{
    show();
}

void FloatingBall::hideBall()
{
    hide();
}

bool FloatingBall::isBallVisible() const
{
    return isVisible();
}

// 绘制事件
void FloatingBall::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (m_appearance.useImage && m_movie && m_movie->isValid()) {
        // 使用GIF动画，完全覆盖悬浮球
        QPixmap currentFrame = m_movie->currentPixmap();
        if (!currentFrame.isNull()) {
            // 将GIF缩放到悬浮球的完整大小
            QPixmap scaledFrame = currentFrame.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            painter.drawPixmap(0, 0, scaledFrame);
        } else {
            qDebug() << "GIF帧为空";
        }
    } else {
        // 使用默认绘制方式
        QColor backgroundColor = m_appearance.backgroundColor;
        if (m_isDragging) {
            backgroundColor = m_appearance.dragColor;
        }
        
        QRect circleRect = rect().adjusted(2, 2, -2, -2);
        
        painter.setBrush(backgroundColor);
        painter.setPen(QPen(m_appearance.borderColor, m_appearance.borderWidth));
        painter.drawEllipse(circleRect);
        
        // 只有在不使用图片时才绘制文字
        if (!m_appearance.text.isEmpty()) {
            painter.setPen(m_appearance.textColor);
            painter.setFont(m_appearance.font);
            painter.drawText(rect(), Qt::AlignCenter, m_appearance.text);
        }
    }
}

void FloatingBall::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        m_showTimer->stop();
        update();
        emit dragStateChanged(true);
    }
}

void FloatingBall::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - m_dragPosition);
        updatePosition();
    }
}

void FloatingBall::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        update();
        emit dragStateChanged(false);
    }
}

void FloatingBall::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    if (!m_isDragging) {
        m_showTimer->start();
    }
    emit mouseEntered();
}

void FloatingBall::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_showTimer->stop();
    emit mouseLeft();
}

void FloatingBall::onShowTimerTimeout()
{
    if (!m_isDragging) {
        showContextMenu(mapToGlobal(QPoint(0, height())));
    }
}

// 菜单项触发槽函数
void FloatingBall::onMenuItemTriggered(QAction *action)
{
    if (action) {
        QString itemText = action->text();
        emit menuItemClicked(itemText);
        
        // 语言模型子菜单处理
        if (itemText == "ChatGPT") {
            // 已选择 ChatGPT 模型
        } else if (itemText == "Claude") {
            // 已选择 Claude 模型
        } else if (itemText == "Gemini") {
            // 已选择 Gemini 模型
        } else if (itemText == "文心一言") {
            // 已选择 文心一言 模型
        } else if (itemText == "通义千问") {
            // 已选择 通义千问 模型
        } else if (itemText == "讯飞星火") {
            // 已选择 讯飞星火 模型
        } else if (itemText == "智谱清言") {
            // 已选择 智谱清言 模型
        } else if (itemText == "自定义模型") {
            QMessageBox::information(this, "自定义模型", "自定义模型配置功能开发中...");
        }
        // 界面外观子菜单处理
        else if (itemText == "默认外观") {
            setTheme(ThemeType::Default);
            // 更新菜单项的选中状态
            updateAppearanceMenuState(ThemeType::Default);
        } else if (itemText == "金小獴") {
            setTheme(ThemeType::JinXiaoMeng);
            // 更新菜单项的选中状态
            updateAppearanceMenuState(ThemeType::JinXiaoMeng);
        }
        // 其他菜单项处理
        else if (itemText == "鼠标随航") {
            QMessageBox::information(this, "鼠标随航", "鼠标随航功能已启用");
        } else if (itemText == "录制回想") {
            QMessageBox::information(this, "录制回想", "录制回想功能开发中...");
        } else if (itemText == "设置") {
            QMessageBox::information(this, "设置", "设置功能开发中...");
        }
    }
}

void FloatingBall::showContextMenu(const QPoint &pos)
{
    if (!m_contextMenu) {
        createMenuItems();
    }
    
    if (m_contextMenu && !m_contextMenu->actions().isEmpty()) {
        m_contextMenu->setStyleSheet(
            "QMenu {"
            "    background-color: #1a1a1a;"
            "    border: 1px solid #333333;"
            "    border-radius: 6px;"
            "    padding: 4px;"
            "    min-width: 160px;"
            "}"
            "QMenu::item {"
            "    background-color: transparent;"
            "    color: #ffffff;"
            "    padding: 6px 12px;"
            "    border-radius: 4px;"
            "    margin: 1px;"
            "    font-size: 12px;"
            "}"
            "QMenu::item:selected {"
            "    background-color: #0064ff;"
            "}"
            "QMenu::separator {"
            "    height: 1px;"
            "    background-color: #333333;"
            "    margin: 4px 0px;"
            "}"
        );
        
        QPoint menuPos = calculateMenuPosition(pos);
        m_menuVisible = true;
        m_contextMenu->exec(menuPos);
        m_menuVisible = false;
    }
}

void FloatingBall::updatePosition()
{
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    QRect widgetGeometry = geometry();
    
    if (widgetGeometry.left() < screenGeometry.left()) {
        move(screenGeometry.left(), y());
    }
    if (widgetGeometry.right() > screenGeometry.right()) {
        move(screenGeometry.right() - width(), y());
    }
    if (widgetGeometry.top() < screenGeometry.top()) {
        move(x(), screenGeometry.top());
    }
    if (widgetGeometry.bottom() > screenGeometry.bottom()) {
        move(x(), screenGeometry.bottom() - height());
    }
}

QPoint FloatingBall::calculateMenuPosition(const QPoint &originalPos)
{
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    
    int menuWidth = 160;
    int menuHeight = 200;
    
    QPoint menuPos = originalPos;
    
    if (menuPos.x() + menuWidth > screenGeometry.right()) {
        menuPos.setX(screenGeometry.right() - menuWidth - 10);
    }
    if (menuPos.x() < screenGeometry.left()) {
        menuPos.setX(screenGeometry.left() + 10);
    }
    if (menuPos.y() + menuHeight > screenGeometry.bottom()) {
        menuPos.setY(menuPos.y() - height() - menuHeight - 5);
    }
    if (menuPos.y() < screenGeometry.top()) {
        menuPos.setY(screenGeometry.top() + 10);
    }
    
    return menuPos;
}

// 创建菜单项
void FloatingBall::createMenuItems()
{
    if (m_contextMenu) {
        delete m_contextMenu;
    }
    
    m_contextMenu = new QMenu(this);
    m_contextMenu->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    
    if (m_menuItems.isEmpty()) {
        // 创建语言模型子菜单
        QList<MenuItemConfig> languageModelSubItems = {
            {"ChatGPT", "", true, true, "OpenAI ChatGPT模型"},
            {"Claude", "", true, false, "Anthropic Claude模型"},
            {"Gemini", "", true, false, "Google Gemini模型"},
            {"文心一言", "", true, false, "百度文心一言模型"},
            {"通义千问", "", true, false, "阿里通义千问模型"},
            {"讯飞星火", "", true, false, "科大讯飞星火模型"},
            {"智谱清言", "", true, false, "智谱AI清言模型"},
            {"", "", false, false, ""}, // 分隔符
            {"自定义模型", "", false, false, "添加自定义模型配置"}
        };
        
        // 创建界面外观子菜单
        QList<MenuItemConfig> appearanceSubItems = {
            {"默认外观", "", true, m_currentTheme == ThemeType::Default, "经典蓝色圆形外观"},
            {"金小獴", "", true, m_currentTheme == ThemeType::JinXiaoMeng, "金色小獴主题外观"}
        };
        
        QList<MenuItemConfig> defaultItems = {
            {"语言模型", "", false, false, "选择语言模型", languageModelSubItems},
            {"界面外观", "", false, false, "选择界面主题", appearanceSubItems},
            {"", "", false, false, ""}, // 分隔符
            {"鼠标随航", "", true, false, "鼠标随航功能"},
            {"录制回想", "", true, false, "录制回想功能"},
            {"", "", false, false, ""}, // 分隔符
            {"设置", "", false, false, "应用设置"}
        };
        m_menuItems = defaultItems;
    }
    
    for (const MenuItemConfig &item : m_menuItems) {
        if (item.text.isEmpty()) {
            m_contextMenu->addSeparator();
        } else {
            QAction *action = new QAction(item.text, m_contextMenu);
            action->setCheckable(item.checkable);
            action->setChecked(item.checked);
            if (!item.tooltip.isEmpty()) {
                action->setToolTip(item.tooltip);
            }
            
            // 如果有子菜单项，创建子菜单
            if (!item.subItems.isEmpty()) {
                QMenu *subMenu = new QMenu(item.text, m_contextMenu);
                subMenu->setStyleSheet(m_contextMenu->styleSheet()); // 继承父菜单样式
                
                for (const MenuItemConfig &subItem : item.subItems) {
                    if (subItem.text.isEmpty()) {
                        subMenu->addSeparator();
                    } else {
                        QAction *subAction = new QAction(subItem.text, subMenu);
                        subAction->setCheckable(subItem.checkable);
                        subAction->setChecked(subItem.checked);
                        if (!subItem.tooltip.isEmpty()) {
                            subAction->setToolTip(subItem.tooltip);
                        }
                        
                        subMenu->addAction(subAction);
                        
                        // 连接子菜单项信号槽
                        connect(subAction, &QAction::triggered, this, [this, subAction]() {
                            onMenuItemTriggered(subAction);
                        });
                    }
                }
                
                action->setMenu(subMenu);
            }
            
            m_contextMenu->addAction(action);
            
            // 连接信号槽
            connect(action, &QAction::triggered, this, [this, action]() {
                onMenuItemTriggered(action);
            });
        }
    }
} 

// 更新动画
void FloatingBall::updateMovie()
{
    // 停止并删除旧的动画
    if (m_movie) {
        m_movie->stop();
        delete m_movie;
        m_movie = nullptr;
    }
    
    // 如果使用图片且路径不为空，创建新的动画
    if (m_appearance.useImage && !m_appearance.imagePath.isEmpty()) {
        m_movie = new QMovie(m_appearance.imagePath, QByteArray(), this);
        if (m_movie->isValid()) {
            // 连接动画帧变化信号到重绘
            connect(m_movie, &QMovie::frameChanged, this, [this]() {
                update();
            });
            m_movie->start();
        }
    }
}

// 更新外观菜单状态
void FloatingBall::updateAppearanceMenuState(ThemeType theme)
{
    if (!m_contextMenu) {
        return;
    }
    
    // 查找外观菜单项
    QList<QAction*> actions = m_contextMenu->actions();
    for (QAction* action : actions) {
        if (action->text() == "界面外观" && action->menu()) {
            QMenu* appearanceMenu = action->menu();
            QList<QAction*> subActions = appearanceMenu->actions();
            
            for (QAction* subAction : subActions) {
                if (subAction->text() == "默认外观") {
                    subAction->setChecked(theme == ThemeType::Default);
                } else if (subAction->text() == "金小獴") {
                    subAction->setChecked(theme == ThemeType::JinXiaoMeng);
                }
            }
            break;
        }
    }
} 