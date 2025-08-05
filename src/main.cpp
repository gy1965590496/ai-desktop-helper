#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include "floatingball.h"
#include "trayicon.h"
#include "screenmonitor.h"
#include "networkmanager.h"
#include "settingsdialog/settingsdialog.h"
#include "settingsdialog/appFilterWidget.h"
#include <QDir> // Added for QDir::currentPath()

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	// 检查系统托盘是否可用
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(nullptr, "错误", "系统托盘不可用，无法启动应用。");
		return 1;
	}

	// 创建悬浮球对象
	FloatingBall* ball = new FloatingBall();

	// 创建托盘图标管理器
	TrayIcon* trayIcon = new TrayIcon(ball);

	// 创建屏幕监控模块
	ScreenMonitor* screenMonitor = new ScreenMonitor();

	// 创建网络管理器
	NetworkManager* networkManager = new NetworkManager();

	// 创建设置界面
	SettingsDialog* settingsDialog = new SettingsDialog();
	
	// 设置 ScreenMonitor 引用
	settingsDialog->setScreenMonitor(screenMonitor);

	// 配置网络管理器
	NetworkConfig networkConfig;
	networkConfig.baseUrl = "https://api.example.com";
	networkConfig.timeout = 30000;
	networkConfig.maxRetries = 3;
	networkConfig.retryDelay = 1000;
	networkConfig.headers["User-Agent"] = "AI-Desktop-Helper/1.0";
	networkManager->setConfig(networkConfig);

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
		[ball, screenMonitor](const QString& oldApp, const QString& newApp) {
			qDebug() << "应用切换:" << oldApp << "->" << newApp;
		});

	QObject::connect(screenMonitor, &ScreenMonitor::appInfoUpdated,
		[ball](const QString& appName, const AppInfo& appInfo) {
			qDebug() << "应用信息更新完成:" << appName;
			// 设置悬浮球左上角图标
			ball->setAppIcon(appInfo.appIcon);
		});

	QObject::connect(screenMonitor, &ScreenMonitor::screenshotCaptured,
		[settingsDialog](const QString& appName, const QPixmap& screenshot) {
			qDebug() << "截图完成:" << appName << "尺寸:" << screenshot.size();

			// 预留：这里可以添加截图处理逻辑
		});

	QObject::connect(screenMonitor, &ScreenMonitor::errorOccurred,
		[](const QString& error) {
			qDebug() << "屏幕监控错误:" << error;
		});

	// 连接网络管理器信号
	QObject::connect(networkManager, &NetworkManager::requestFinished,
		[](const NetworkResponse& response) {
			qDebug() << "网络请求完成，状态码:" << response.statusCode;
		});

	QObject::connect(networkManager, &NetworkManager::requestError,
		[](const QString& error) {
			qDebug() << "网络请求错误:" << error;
		});

	// 连接设置界面信号
	QObject::connect(settingsDialog, &SettingsDialog::excludedAppsChanged,
		[screenMonitor](const QStringList& excludedApps) {
			qDebug() << "不监控应用名单已更新:" << excludedApps;
			// 这里可以将不监控应用名单传递给屏幕监控模块
		});

	// 连接应用过滤信号
	QObject::connect(settingsDialog, &SettingsDialog::appFiltersChanged,
		[screenMonitor](const QStringList& filteredApps) {
			qDebug() << "应用过滤列表已更新:" << filteredApps;
			
			// 清除现有的应用过滤器
			QStringList currentFilters = screenMonitor->getAppFilters();
			for (const QString& filter : currentFilters) {
				screenMonitor->removeAppFilter(filter);
			}
			
			// 添加新的应用过滤器（排除这些应用）
			for (const QString& appName : filteredApps) {
				screenMonitor->addAppFilter(appName, true); // true 表示排除
			}
			
			qDebug() << "已更新 ScreenMonitor 的应用过滤器";
		});

	// 连接记录管理信号
	QObject::connect(screenMonitor, &ScreenMonitor::appRecordAdded,
		[settingsDialog](const AppRecord& record) {
			settingsDialog->addAppRecord(record);
		});

	QObject::connect(screenMonitor, &ScreenMonitor::appRecordsCleared,
		[settingsDialog]() {
			settingsDialog->clearAppRecords();
		});

	// 连接信号槽
	QObject::connect(trayIcon, &TrayIcon::exitRequested, [&app]() {
		app.quit();
		});

	// 连接悬浮球菜单项点击信号（预留扩展点）
	QObject::connect(ball, &FloatingBall::menuItemClicked, [screenMonitor, settingsDialog](const QString& itemText) {
		// 处理菜单项点击
		if (itemText == "屏幕监控") {
			// 启动/停止屏幕监控
			if (screenMonitor->isMonitoring()) {
				screenMonitor->stopMonitoring();
				qDebug() << "屏幕监控已停止";
			}
			else {
				screenMonitor->startMonitoring();
				qDebug() << "屏幕监控已启动";
			}
		}
		else if (itemText == "设置") {
			// 显示设置界面
			if (settingsDialog->isVisible()) {
				settingsDialog->hide();
			}
			else {
				settingsDialog->show();
				settingsDialog->raise();
				settingsDialog->activateWindow();
			}
		}
		else {
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

	// 显示托盘图标
	trayIcon->show();

	// 启动屏幕监控（可选，默认不启动）
	// screenMonitor->startMonitoring();

	// 运行应用程序
	return app.exec();
}