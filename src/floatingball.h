#ifndef FLOATINGBALL_H
#define FLOATINGBALL_H

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QColor>
#include <QFont>
#include <QMovie>
#include <QIcon> // Added for QIcon

// 前向声明
class QMenu;
class QAction;

// 主题枚举
enum class ThemeType {
    Default,    // 默认主题
    JinXiaoMeng // 金小獴主题
};

// 悬浮球外观配置结构体
struct FloatingBallAppearance {
    QColor backgroundColor = QColor(0, 100, 255);    // 背景色
    QColor borderColor = QColor(255, 255, 255);      // 边框色
    int borderWidth = 2;                              // 边框宽度
    int size = 60;                                    // 大小
    QString text = "K";                               // 显示文字
    QFont font = QFont("Arial", 16, QFont::Bold);    // 字体
    QColor textColor = QColor(255, 255, 255);        // 文字颜色
    QColor dragColor = QColor(0, 80, 200);           // 拖拽时的颜色
    QString imagePath;                                // 图片路径（用于GIF等）
    bool useImage = false;                            // 是否使用图片
};

// 菜单项配置结构体
struct MenuItemConfig {
    QString text;                                     // 菜单项文字
    QString icon;                                     // 图标路径（预留）
    bool checkable = false;                           // 是否可勾选
    bool checked = false;                             // 是否已勾选
    QString tooltip;                                  // 提示文字
    QList<MenuItemConfig> subItems;                   // 子菜单项列表
};

// 悬浮球类
class FloatingBall : public QWidget
{
    Q_OBJECT

public:
    // 构造函数
    explicit FloatingBall(QWidget *parent = nullptr);
    
    // 外观相关方法
    void setAppearance(const FloatingBallAppearance &appearance);
    FloatingBallAppearance getAppearance() const;
    
    // 主题相关方法
    void setTheme(ThemeType theme);
    ThemeType getCurrentTheme() const;
    
    // 位置相关方法
    void moveToTrayArea();
    void moveToScreenCenter();
    
    // 菜单相关方法
    void setMenuItems(const QList<MenuItemConfig> &items);
    void addMenuItem(const MenuItemConfig &item);
    void clearMenuItems();
    
    // 显示/隐藏控制
    void showBall();
    void hideBall();
    bool isBallVisible() const;

    void setAppIcon(const QIcon &icon);
    QIcon getAppIcon() const;

protected:
    // 事件处理
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    // 菜单显示定时器槽函数
    void onShowTimerTimeout();
    
    // 菜单项响应槽函数（预留）
    void onMenuItemTriggered(QAction *action);

signals:
    // 菜单项点击信号
    void menuItemClicked(const QString &itemText);
    
    // 拖拽状态变化信号
    void dragStateChanged(bool isDragging);
    
    // 鼠标进入/离开信号
    void mouseEntered();
    void mouseLeft();

private:
    // 私有方法
    void initializeUI();
    void setupTimer();
    void showContextMenu(const QPoint &pos);
    void updatePosition();
    QPoint calculateMenuPosition(const QPoint &originalPos);
    void createMenuItems();
    void updateMovie();
    void updateAppearanceMenuState(ThemeType theme);
    
    // 成员变量
    QPoint m_dragPosition;                    // 拖拽位置
    bool m_isDragging;                        // 是否正在拖拽
    QTimer *m_showTimer;                      // 显示菜单的定时器
    
    // 外观配置
    FloatingBallAppearance m_appearance;      // 外观配置
    ThemeType m_currentTheme;                 // 当前主题
    
    // 动画相关
    QMovie *m_movie;                          // GIF动画对象
    
    // 菜单相关
    QList<MenuItemConfig> m_menuItems;        // 菜单项配置列表
    QMenu *m_contextMenu;                     // 右键菜单对象
    bool m_menuVisible;                       // 菜单是否正在显示

    QIcon m_appIcon; // 当前激活应用的图标
};

#endif // FLOATINGBALL_H 