#include "AppFilterWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>

AppFilterWidget::AppFilterWidget(QWidget* parent)
	: QWidget(parent)
{
	_filterEdit = new QLineEdit(this);
	_filterEdit->setPlaceholderText("输入关键词过滤应用名...");

	_appListWidget = new QListWidget(this);

	_addButton = new QPushButton("+", this);
	_removeButton = new QPushButton("-", this);

	auto* buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(_addButton);
	buttonLayout->addWidget(_removeButton);
	buttonLayout->addStretch();

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(_filterEdit);
	layout->addWidget(_appListWidget);
	layout->addLayout(buttonLayout);
	setLayout(layout);

	connect(_addButton, &QPushButton::clicked, this, &AppFilterWidget::onAddAppClicked);
	connect(_removeButton, &QPushButton::clicked, this, &AppFilterWidget::onRemoveAppClicked);
	connect(_filterEdit, &QLineEdit::textChanged, this, &AppFilterWidget::onFilterTextChanged);
	
	// 添加一些默认的系统应用过滤器
	addDefaultSystemApps();
}

QStringList AppFilterWidget::getFilteredApps() const
{
	QStringList filteredApps;
	for (const QString& path : _allAppPaths) {
		QFileInfo fi(path);
		filteredApps.append(fi.fileName());
	}
	return filteredApps;
}

void AppFilterWidget::setFilteredApps(const QStringList& apps)
{
	_allAppPaths.clear();
	for (const QString& appName : apps) {
		// 如果传入的是应用名（如 chrome.exe），直接添加
		if (appName.endsWith(".exe")) {
			_allAppPaths.append(appName);
		} else {
			// 如果传入的是完整路径，添加路径
			_allAppPaths.append(appName);
		}
	}
	refreshList();
}

void AppFilterWidget::onAddAppClicked()
{
	QString filter = "Executable Files (*.exe)";
	QString filePath = QFileDialog::getOpenFileName(this, "选择应用程序 (.exe)", QString(), filter);
	if (!filePath.isEmpty() && !_allAppPaths.contains(filePath)) {
		_allAppPaths.append(filePath);
		refreshList();
		emitFilteredAppsChanged();
		qDebug() << "添加应用过滤器:" << QFileInfo(filePath).fileName();
	}
}

void AppFilterWidget::onRemoveAppClicked()
{
	auto* item = _appListWidget->currentItem();
	if (item) {
		QString filePath = item->data(Qt::UserRole).toString();
		_allAppPaths.removeAll(filePath);
		refreshList();
		emitFilteredAppsChanged();
		qDebug() << "移除应用过滤器:" << QFileInfo(filePath).fileName();
	}
}

void AppFilterWidget::onFilterTextChanged(const QString& text)
{
	refreshList();
}

void AppFilterWidget::refreshList()
{
	_appListWidget->clear();
	QString keyword = _filterEdit->text();

	for (const QString& path : _allAppPaths) {
		QFileInfo fi(path);
		QString fileName = fi.fileName();  // 获取 exe 文件名（如 chrome.exe）

		if (fileName.contains(keyword, Qt::CaseInsensitive)) {
			auto* item = new QListWidgetItem(fileName);
			item->setData(Qt::UserRole, path);  // 存路径，后续可用
			_appListWidget->addItem(item);
		}
	}
}

void AppFilterWidget::emitFilteredAppsChanged()
{
	emit filteredAppsChanged(getFilteredApps());
}

void AppFilterWidget::addDefaultSystemApps()
{
	// 添加一些常见的系统应用作为默认过滤器
	QStringList defaultSystemApps = {
		"explorer.exe",
		"dwm.exe", 
		"taskmgr.exe",
		"svchost.exe",
		"csrss.exe",
		"winlogon.exe",
	};
	
	for (const QString& appName : defaultSystemApps) {
		if (!_allAppPaths.contains(appName)) {
			_allAppPaths.append(appName);
		}
	}
	
	refreshList();
	emitFilteredAppsChanged();
}
