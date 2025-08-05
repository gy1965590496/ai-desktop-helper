#include "screenmonitor.h"
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QScreen>
#include <QApplication>
#include <QWindow>
#include <QFileInfo>
#include <QIcon>
#include <QFile>
#include <QTextStream>
#include <QStyle> // 添加QStyle头文件

// Windows API 头文件
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <shellapi.h>

ScreenMonitor::ScreenMonitor(QObject *parent)
    : QObject(parent)
    , m_appCheckTimer(nullptr)
    , m_screenshotTimer(nullptr)
    , m_currentActiveApp("")
    , m_lastActiveApp("")
    , m_isMonitoring(false)
    , m_screenshotCounter(0)
    , m_floatingBall(nullptr)
{
    initializeMonitoring();
}

ScreenMonitor::~ScreenMonitor()
{
    stopMonitoring();
}

void ScreenMonitor::initializeMonitoring()
{
    // 初始化定时器
    m_appCheckTimer = new QTimer(this);
    m_appCheckTimer->setInterval(1000); // 每秒检查一次激活应用
    connect(m_appCheckTimer, &QTimer::timeout, this, &ScreenMonitor::checkActiveApplication);
    
    m_screenshotTimer = new QTimer(this);
    connect(m_screenshotTimer, &QTimer::timeout, this, &ScreenMonitor::captureScreenshot);
    
    // 创建保存目录
    createSaveDirectory();
    
    // 添加一些默认的应用过滤器
    addAppFilter("explorer.exe", true);  // 排除资源管理器
    addAppFilter("dwm.exe", true);       // 排除桌面窗口管理器
    addAppFilter("taskmgr.exe", true);   // 排除任务管理器
}

void ScreenMonitor::startMonitoring()
{
    if (m_isMonitoring) {
        return;
    }
    
    m_isMonitoring = true;
    m_appCheckTimer->start();
    m_screenshotTimer->start(m_config.captureInterval);
    
    qDebug() << "Screen monitoring started";
}

void ScreenMonitor::stopMonitoring()
{
    if (!m_isMonitoring) {
        return;
    }
    
    m_isMonitoring = false;
    m_appCheckTimer->stop();
    m_screenshotTimer->stop();
    
    qDebug() << "Screen monitoring stopped";
}

bool ScreenMonitor::isMonitoring() const
{
    return m_isMonitoring;
}

void ScreenMonitor::setConfig(const ScreenshotConfig &config)
{
    m_config = config;
    
    // 如果正在监控，更新截图定时器间隔
    if (m_isMonitoring && m_screenshotTimer) {
        m_screenshotTimer->setInterval(m_config.captureInterval);
    }
    
    // 创建保存目录
    createSaveDirectory();
}

ScreenshotConfig ScreenMonitor::getConfig() const
{
    return m_config;
}

void ScreenMonitor::addAppFilter(const QString &appName, bool exclude)
{
    m_appFilters[appName.toLower()] = exclude;
}

void ScreenMonitor::removeAppFilter(const QString &appName)
{
    m_appFilters.remove(appName.toLower());
}

QStringList ScreenMonitor::getAppFilters() const
{
    return m_appFilters.keys();
}

QPixmap ScreenMonitor::captureCurrentWindow()
{
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return QPixmap();
    }
    
    // 获取窗口矩形
    RECT rect;
    GetWindowRect(hwnd, &rect);
    
    // 使用Qt截图
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) {
        return QPixmap();
    }
    
    QRect windowRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
    return screen->grabWindow(0, windowRect.x(), windowRect.y(), windowRect.width(), windowRect.height());
}

QPixmap ScreenMonitor::captureFullScreen()
{
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) {
        return QPixmap();
    }
    
    return screen->grabWindow(0);
}

void ScreenMonitor::checkActiveApplication()
{
    QString currentApp = getActiveApplication();
    
    if (currentApp != m_currentActiveApp) {
        QString oldApp = m_currentActiveApp;
        m_currentActiveApp = currentApp;
        
        // 更新新应用的信息
        if (!currentApp.isEmpty()) {
            updateAppInfo(currentApp);
        }
        
        // 发送应用切换信号
        emit activeApplicationChanged(oldApp, currentApp);
        
        qDebug() << "Active application changed from" << oldApp << "to" << currentApp;
    }
}

void ScreenMonitor::captureScreenshot()
{
    if (!m_isMonitoring || m_currentActiveApp.isEmpty()) {
        return;
    }
    
    // 检查是否应该截图这个应用
    if (!shouldCaptureApp(m_currentActiveApp)) {
        return;
    }
    
    // 截图当前窗口
    QPixmap screenshot = captureCurrentWindow();
    if (screenshot.isNull()) {
        emit errorOccurred("Failed to capture screenshot");
        return;
    }
    
    // 更新应用缓存
    if (!m_appCache.contains(m_currentActiveApp)) {
        AppInfo appInfo;
        appInfo.processName = m_currentActiveApp;
        appInfo.windowTitle = ""; // 可以后续获取
        m_appCache[m_currentActiveApp] = appInfo;
    }
    
    m_appCache[m_currentActiveApp].lastScreenshot = screenshot;
    m_appCache[m_currentActiveApp].lastCaptureTime = QDateTime::currentDateTime();
    
    // 创建应用记录
    AppRecord record;
    record.appName = m_currentActiveApp;
    record.timestamp = QDateTime::currentDateTime();
    record.screenshot = screenshot;
    record.appPath = m_appCache[m_currentActiveApp].executablePath;
    record.windowTitle = m_appCache[m_currentActiveApp].windowTitle;
    
    // 添加到记录列表
    m_appRecords.append(record);
    
    // 限制记录数量，保持最新的100条记录
    if (m_appRecords.size() > 100) {
        m_appRecords.removeFirst();
    }
    
    // 自动保存
    if (m_config.autoSave) {
        saveScreenshot(screenshot, m_currentActiveApp);
    }
    
    // 清理旧缓存
    if (m_appCache.size() > m_config.maxCacheSize) {
        cleanupOldScreenshots();
    }
    
    // 发送截图完成信号
    emit screenshotCaptured(m_currentActiveApp, screenshot);
    
    // 发送记录添加信号
    emit appRecordAdded(record);
    
    m_screenshotCounter++;
    qDebug() << "Screenshot captured for" << m_currentActiveApp << "(" << m_screenshotCounter << ")";
}

QString ScreenMonitor::getActiveApplication()
{
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return "";
    }
    
    return getProcessNameFromWindow(hwnd);
}

QString ScreenMonitor::getProcessNameFromWindow(HWND hwnd)
{
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    
    if (processId == 0) {
        return "";
    }
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) {
        return "";
    }
    
    char processName[MAX_PATH];
    if (GetModuleFileNameExA(hProcess, NULL, processName, MAX_PATH)) {
        CloseHandle(hProcess);
        QFileInfo fileInfo(QString::fromLocal8Bit(processName));
        return fileInfo.fileName();
    }
    
    CloseHandle(hProcess);
    return "";
}

QString ScreenMonitor::getExecutablePathFromProcess(DWORD processId) const
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) {
        return "";
    }
    
    char processPath[MAX_PATH];
    if (GetModuleFileNameExA(hProcess, NULL, processPath, MAX_PATH)) {
        CloseHandle(hProcess);
        return QString::fromLocal8Bit(processPath);
    }
    
    CloseHandle(hProcess);
    return "";
}

bool ScreenMonitor::shouldCaptureApp(const QString &appName)
{
    QString lowerAppName = appName.toLower();
    
    // 检查是否在过滤列表中
    if (m_appFilters.contains(lowerAppName)) {
        return !m_appFilters[lowerAppName]; // 如果exclude为true，则不截图
    }
    
    return true; // 默认截图
}

void ScreenMonitor::saveScreenshot(const QPixmap &screenshot, const QString &appName)
{
    if (screenshot.isNull()) {
        return;
    }
    
    // 生成文件名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString fileName = QString("%1_%2.jpg").arg(appName).arg(timestamp);
    QString filePath = m_config.savePath + fileName;
    
    // 保存图片
    if (screenshot.save(filePath, "JPEG", m_config.imageQuality)) {
        qDebug() << "Screenshot saved:" << filePath;
    } else {
        emit errorOccurred("Failed to save screenshot: " + filePath);
    }
}

void ScreenMonitor::cleanupOldScreenshots()
{
    // 简单的清理策略：删除最旧的应用缓存
    if (m_appCache.size() <= m_config.maxCacheSize) {
        return;
    }
    
    // 找到最旧的应用
    QString oldestApp;
    QDateTime oldestTime;
    bool first = true;
    
    for (auto it = m_appCache.begin(); it != m_appCache.end(); ++it) {
        if (first || it.value().lastCaptureTime < oldestTime) {
            oldestTime = it.value().lastCaptureTime;
            oldestApp = it.key();
            first = false;
        }
    }
    
    if (!oldestApp.isEmpty()) {
        m_appCache.remove(oldestApp);
        qDebug() << "Removed old app cache:" << oldestApp;
    }
}

void ScreenMonitor::createSaveDirectory()
{
    QDir dir(m_config.savePath);
    if (!dir.exists()) {
        if (dir.mkpath(".")) {
            qDebug() << "Created screenshot directory:" << m_config.savePath;
        } else {
            emit errorOccurred("Failed to create screenshot directory: " + m_config.savePath);
        }
    }
}

// 设置悬浮球引用
void ScreenMonitor::setFloatingBall(QWidget *ball)
{
    m_floatingBall = ball;
}

// 获取当前应用信息
AppInfo ScreenMonitor::getCurrentAppInfo() const
{
    if (m_currentActiveApp.isEmpty()) {
        return AppInfo();
    }
    
    return m_appCache.value(m_currentActiveApp, AppInfo());
}

// 获取窗口标题
QString ScreenMonitor::getWindowTitle() const
{
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return "";
    }
    
    return getWindowTitleFromWindow(hwnd);
}

// 获取应用图标
QIcon ScreenMonitor::getAppIcon() const
{
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return QIcon();
    }
    
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    
    if (processId == 0) {
        return QIcon();
    }
    
    return getAppIconFromProcess(processId);
}

// 获取应用版本
QString ScreenMonitor::getAppVersion() const
{
    QString executablePath = getExecutablePathFromProcess(GetCurrentProcessId());
    return getAppVersionFromPath(executablePath);
}

// 从窗口获取窗口标题
QString ScreenMonitor::getWindowTitleFromWindow(HWND hwnd) const
{
    if (!hwnd) {
        return "";
    }
    
    int length = GetWindowTextLengthW(hwnd);
    if (length == 0) {
        return "";
    }
    
    wchar_t *buffer = new wchar_t[length + 1];
    GetWindowTextW(hwnd, buffer, length + 1);
    
    QString title = QString::fromWCharArray(buffer);
    delete[] buffer;
    
    return title;
}

// 从进程获取应用图标
QIcon ScreenMonitor::getAppIconFromProcess(DWORD processId) const
{
    QString executablePath = getExecutablePathFromProcess(processId);
    return getAppIconFromPath(executablePath);
}

// 从路径获取应用图标
QIcon ScreenMonitor::getAppIconFromPath(const QString &executablePath) const
{
    if (executablePath.isEmpty()) {
        qDebug() << "可执行文件路径为空";
        return QIcon();
    }
    
    qDebug() << "尝试获取图标，路径:" << executablePath;
    
    // 方法1：使用Windows API从可执行文件中提取图标
    SHFILEINFOW shfi = {0};
    DWORD_PTR result = SHGetFileInfoW(
        reinterpret_cast<const wchar_t*>(executablePath.utf16()),
        0,
        &shfi,
        sizeof(shfi),
        SHGFI_ICON | SHGFI_LARGEICON
    );
    
    if (result && shfi.hIcon) {
        qDebug() << "Windows API获取图标成功";
        
        // 获取图标信息
        ICONINFO iconInfo;
        if (GetIconInfo(shfi.hIcon, &iconInfo)) {
            // 获取位图信息
            BITMAP bm;
            if (GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bm)) {
                qDebug() << "图标位图尺寸:" << bm.bmWidth << "x" << bm.bmHeight;
                
                // 创建DC
                HDC hdc = CreateCompatibleDC(NULL);
                HGDIOBJ oldBitmap = SelectObject(hdc, iconInfo.hbmColor);
                
                // 创建位图信息头
                BITMAPINFO bmi = {0};
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = bm.bmWidth;
                bmi.bmiHeader.biHeight = -bm.bmHeight; // 负值表示自上而下
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;
                
                // 分配内存存储位图数据
                int bufferSize = bm.bmWidth * bm.bmHeight * 4;
                BYTE* buffer = new BYTE[bufferSize];
                
                // 获取位图数据
                if (GetDIBits(hdc, iconInfo.hbmColor, 0, bm.bmHeight, buffer, &bmi, DIB_RGB_COLORS)) {
                    // 创建QImage
                    QImage image(buffer, bm.bmWidth, bm.bmHeight, QImage::Format_ARGB32);
                    QImage copiedImage = image.copy(); // 复制图像数据
                    
                    // 创建QPixmap和QIcon
                    QPixmap pixmap = QPixmap::fromImage(copiedImage);
                    if (!pixmap.isNull()) {
                        QIcon icon(pixmap);
                        qDebug() << "成功提取应用图标，尺寸:" << pixmap.size();
                        
                        delete[] buffer;
                        SelectObject(hdc, oldBitmap);
                        DeleteDC(hdc);
                        DeleteObject(iconInfo.hbmColor);
                        DeleteObject(iconInfo.hbmMask);
                        DestroyIcon(shfi.hIcon);
                        return icon;
                    }
                }
                
                delete[] buffer;
                SelectObject(hdc, oldBitmap);
                DeleteDC(hdc);
            }
            DeleteObject(iconInfo.hbmColor);
            DeleteObject(iconInfo.hbmMask);
        }
        
        DestroyIcon(shfi.hIcon);
    } else {
        qDebug() << "Windows API获取图标失败";
    }
    
    // 方法2：使用系统默认图标作为备用
    qDebug() << "使用系统默认图标";
    QFileInfo fileInfo(executablePath);
    QString extension = fileInfo.suffix().toLower();
    
    // 根据文件扩展名获取对应的系统图标
    QStyle::StandardPixmap standardIcon = QStyle::SP_FileIcon;
    if (extension == "exe") {
        standardIcon = QStyle::SP_ComputerIcon;
    } else if (extension == "lnk") {
        standardIcon = QStyle::SP_FileDialogStart;
    } else if (extension == "dll") {
        standardIcon = QStyle::SP_FileDialogDetailedView;
    }
    
    QIcon systemIcon = QApplication::style()->standardIcon(standardIcon);
    if (!systemIcon.isNull()) {
        qDebug() << "使用系统图标成功";
        return systemIcon;
    }
    
    // 方法3：创建默认图标
    qDebug() << "创建默认图标";
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::blue);
    QIcon tempIcon(pixmap);
    return tempIcon;
}

// 从路径获取应用版本
QString ScreenMonitor::getAppVersionFromPath(const QString &executablePath) const
{
    if (executablePath.isEmpty()) {
        return "";
    }
    
    DWORD dwHandle = 0;
    DWORD dwSize = GetFileVersionInfoSizeW(
        reinterpret_cast<const wchar_t*>(executablePath.utf16()),
        &dwHandle
    );
    
    if (dwSize == 0) {
        return "";
    }
    
    QByteArray buffer(dwSize, 0);
    if (!GetFileVersionInfoW(
        reinterpret_cast<const wchar_t*>(executablePath.utf16()),
        dwHandle,
        dwSize,
        buffer.data())) {
        return "";
    }
    
    VS_FIXEDFILEINFO *pFileInfo = nullptr;
    UINT len = 0;
    if (!VerQueryValueW(buffer.data(), L"\\", (void**)&pFileInfo, &len)) {
        return "";
    }
    
    if (len == 0) {
        return "";
    }
    
    DWORD major = (pFileInfo->dwFileVersionMS >> 16) & 0xFFFF;
    DWORD minor = (pFileInfo->dwFileVersionMS >> 0) & 0xFFFF;
    DWORD build = (pFileInfo->dwFileVersionLS >> 16) & 0xFFFF;
    DWORD revision = (pFileInfo->dwFileVersionLS >> 0) & 0xFFFF;
    
    return QString("%1.%2.%3.%4").arg(major).arg(minor).arg(build).arg(revision);
}

// 判断是否为系统应用
bool ScreenMonitor::isSystemApplication(const QString &appName) const
{
    QStringList systemApps = {
        "explorer.exe", "dwm.exe", "taskmgr.exe", "svchost.exe",
        "csrss.exe", "winlogon.exe", "services.exe", "lsass.exe",
        "wininit.exe", "spoolsv.exe", "rundll32.exe", "dllhost.exe"
    };
    
    return systemApps.contains(appName.toLower());
}

// 更新应用信息
void ScreenMonitor::updateAppInfo(const QString &appName)
{
    if (appName.isEmpty()) {
        return;
    }
    
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return;
    }
    
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    
    AppInfo &appInfo = m_appCache[appName];
    appInfo.processName = appName;
    appInfo.windowTitle = getWindowTitleFromWindow(hwnd);
    appInfo.executablePath = getExecutablePathFromProcess(processId);
    appInfo.processId = processId;
    appInfo.isSystemApp = isSystemApplication(appName);
    appInfo.appVersion = getAppVersionFromPath(appInfo.executablePath);
    appInfo.appIcon = getAppIconFromProcess(processId);
    
    // 设置应用描述
    if (appInfo.isSystemApp) {
        appInfo.appDescription = "系统应用";
    } else {
        appInfo.appDescription = "用户应用";
    }
    
    qDebug() << "应用信息更新:" << appName;
    qDebug() << "  窗口标题:" << appInfo.windowTitle;
    qDebug() << "  进程ID:" << appInfo.processId;
    qDebug() << "  版本:" << appInfo.appVersion;
    qDebug() << "  路径:" << appInfo.executablePath;
    
    // 发送应用信息更新信号
    emit appInfoUpdated(appName, appInfo);
} 

// 记录管理方法
QList<AppRecord> ScreenMonitor::getAppRecords() const
{
    return m_appRecords;
}

void ScreenMonitor::clearAppRecords()
{
    m_appRecords.clear();
    emit appRecordsCleared();
    qDebug() << "应用记录已清空";
}

void ScreenMonitor::exportAppRecords(const QString &filePath)
{
    // 这里可以实现导出记录到文件的功能
    // 暂时只是调试输出
    qDebug() << "导出应用记录到:" << filePath;
    qDebug() << "记录数量:" << m_appRecords.size();
} 