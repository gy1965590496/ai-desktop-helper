#ifndef SCREENMONITOR_H
#define SCREENMONITOR_H

#include <QObject>
#include <QTimer>
#include <QPixmap>
#include <QString>
#include <QMap>
#include <QProcess>
#include <QScreen>
#include <QApplication>
#include <QDateTime>
#include <QIcon> // Added for QIcon
#include "common.h" // Added for AppRecord

// Windows API 前向声明
#ifdef _WIN32
#include <windows.h>
#endif

// 应用信息结构体
struct AppInfo {
    QString processName;      // 进程名
    QString windowTitle;      // 窗口标题
    QString executablePath;   // 可执行文件路径
    QPixmap lastScreenshot;  // 最后截图
    QDateTime lastCaptureTime; // 最后截图时间
    QIcon appIcon;           // 应用图标
    QString appVersion;      // 应用版本
    QString appDescription;  // 应用描述
    DWORD processId;         // 进程ID
    bool isSystemApp;        // 是否系统应用
};

// 截图配置结构体
struct ScreenshotConfig {
    int captureInterval = 5000;    // 截图间隔（毫秒）
    int imageQuality = 85;         // 图像质量（1-100）
    QString savePath = "./screenshots/"; // 保存路径
    bool autoSave = false;         // 是否自动保存
    int maxCacheSize = 100;        // 最大缓存数量
};

class ScreenMonitor : public QObject
{
    Q_OBJECT

public:
    explicit ScreenMonitor(QObject *parent = nullptr);
    ~ScreenMonitor();

    // 控制方法
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;

    // 配置方法
    void setConfig(const ScreenshotConfig &config);
    ScreenshotConfig getConfig() const;

    // 应用管理
    void addAppFilter(const QString &appName, bool exclude = true);
    void removeAppFilter(const QString &appName);
    QStringList getAppFilters() const;

    // 记录管理
    QList<AppRecord> getAppRecords() const;
    void clearAppRecords();
    void exportAppRecords(const QString &filePath);

    // 手动截图
    QPixmap captureCurrentWindow();
    QPixmap captureFullScreen();
    
    // 设置悬浮球引用（用于截图时隐藏）
    void setFloatingBall(QWidget *ball);
    
    // 获取应用详细信息
    AppInfo getCurrentAppInfo() const;
    QString getWindowTitle() const;
    QIcon getAppIcon() const;
    QString getAppVersion() const;

signals:
    // 应用切换信号
    void activeApplicationChanged(const QString &oldApp, const QString &newApp);
    
    // 应用信息更新完成信号
    void appInfoUpdated(const QString &appName, const AppInfo &appInfo);
    
    // 截图完成信号
    void screenshotCaptured(const QString &appName, const QPixmap &screenshot);
    
    // 记录管理信号
    void appRecordAdded(const AppRecord &record);
    void appRecordsCleared();
    
    // 错误信号
    void errorOccurred(const QString &error);

private slots:
    // 定时器槽函数
    void checkActiveApplication();
    void captureScreenshot();

private:
    // 私有方法
    void initializeMonitoring();
    QString getActiveApplication();
    QString getProcessNameFromWindow(HWND hwnd);
    QString getExecutablePathFromProcess(DWORD processId) const;
    bool shouldCaptureApp(const QString &appName);
    void saveScreenshot(const QPixmap &screenshot, const QString &appName);
    void cleanupOldScreenshots();
    void createSaveDirectory();
    
    // 新增的私有方法
    QString getWindowTitleFromWindow(HWND hwnd) const;
    QIcon getAppIconFromProcess(DWORD processId) const;
    QIcon getAppIconFromPath(const QString &executablePath) const;
    QString getAppVersionFromPath(const QString &executablePath) const;
    bool isSystemApplication(const QString &appName) const;
    void updateAppInfo(const QString &appName);

    // 成员变量
    QTimer *m_appCheckTimer;           // 应用检测定时器
    QTimer *m_screenshotTimer;         // 截图定时器
    
    QString m_currentActiveApp;        // 当前激活应用
    QString m_lastActiveApp;           // 上次激活应用
    
    ScreenshotConfig m_config;         // 配置信息
    QMap<QString, AppInfo> m_appCache; // 应用缓存
    
    QMap<QString, bool> m_appFilters;  // 应用过滤器（true=排除）
    
    QList<AppRecord> m_appRecords;     // 应用记录列表
    
    bool m_isMonitoring;               // 是否正在监控
    int m_screenshotCounter;           // 截图计数器
    
    QWidget *m_floatingBall;           // 悬浮球引用
};

#endif // SCREENMONITOR_H 