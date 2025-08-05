#include "settingsdialog.h"
#include <QDebug>
#include <QApplication>
#include <QMouseEvent>

// NavItemWidget实现
NavItemWidget::NavItemWidget(const QString& iconPath, const QString& text, QWidget* parent)
	: QWidget(parent), _iconPath(iconPath), _text(text), _hovered(false), _selected(false)
{
	setAttribute(Qt::WA_Hover);
	setMouseTracking(true);

	_iconLabel = new QLabel;
	_iconLabel->setPixmap(QPixmap(iconPath).scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	_textLabel = new QLabel(text);
	_textLabel->setStyleSheet("font-size: 14px;");

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(_iconLabel);
	layout->addWidget(_textLabel);
	layout->setContentsMargins(10, 5, 10, 5);
	layout->setSpacing(10);

	updateStyle();
}

void NavItemWidget::setSelected(bool selected)
{
	_selected = selected;
	updateStyle();
}

void NavItemWidget::enterEvent(QEvent* event)
{
	Q_UNUSED(event);
	_hovered = true;
	updateStyle();
}

void NavItemWidget::leaveEvent(QEvent* event)
{
	Q_UNUSED(event);
	_hovered = false;
	updateStyle();
}

void NavItemWidget::mousePressEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	emit clicked();
}

void NavItemWidget::updateStyle()
{
	if (_selected) {
		setStyleSheet("background-color: #0064ff; border-radius: 6px;");
		_textLabel->setStyleSheet("font-size: 14px; color: white;");
	}
	else if (_hovered) {
		setStyleSheet("background-color: #404040; border-radius: 6px;");
		_textLabel->setStyleSheet("font-size: 14px; color: #ffffff;");
	}
	else {
		setStyleSheet("background-color: transparent;");
		_textLabel->setStyleSheet("font-size: 14px; color: #cccccc;");
	}
}

// SettingsDialog实现
SettingsDialog::SettingsDialog(QWidget* parent)
	: QDialog(parent)
	, m_isDragging(false)
	, m_currentNavIndex(0)
{
	setWindowTitle("设置 - AI桌面助手");
	setFixedSize(1000, 700);

	// 设置窗口标志，去除默认标题栏
	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

	// 设置统一的暗色主题样式
	setStyleSheet(
		"QDialog { background-color: #1a1a1a; color: #ffffff; border: 2px solid #333333; border-radius: 12px; }"
		"QListWidget { background-color: #2a2a2a; border: 1px solid #333333; border-radius: 8px; color: #ffffff; selection-background-color: #0064ff; outline: none; }"
		"QListWidget::item { padding: 12px; border-bottom: 1px solid #333333; border-radius: 4px; margin: 2px; }"
		"QListWidget::item:hover { background-color: #404040; }"
		"QListWidget::item:selected { background-color: #0064ff; color: #ffffff; }"
		"QLineEdit { background-color: #2a2a2a; border: 2px solid #333333; border-radius: 8px; color: #ffffff; padding: 12px; font-size: 14px; }"
		"QLineEdit:focus { border-color: #0064ff; outline: none; }"
		"QPushButton { background-color: #0064ff; color: #ffffff; border: none; padding: 12px 24px; border-radius: 8px; font-weight: bold; font-size: 14px; }"
		"QPushButton:hover { background-color: #0052cc; }"
		"QPushButton:pressed { background-color: #004499; }"
		"QPushButton:disabled { background-color: #404040; color: #808080; }"
		"QLabel { color: #ffffff; font-size: 14px; }"
		"QSlider::groove:horizontal { border: 1px solid #333333; height: 8px; background-color: #2a2a2a; border-radius: 4px; margin: 2px 0; }"
		"QSlider::handle:horizontal { background-color: #0064ff; border: 2px solid #0064ff; width: 20px; margin: -6px 0; border-radius: 10px; }"
		"QSlider::handle:horizontal:hover { background-color: #0052cc; border-color: #0052cc; }"
		"QScrollBar:vertical { background-color: #2a2a2a; width: 12px; border-radius: 6px; }"
		"QScrollBar::handle:vertical { background-color: #404040; border-radius: 6px; min-height: 20px; }"
		"QScrollBar::handle:vertical:hover { background-color: #0064ff; }"
		"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
	);

	initializeUI();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::initializeUI()
{
	// 主布局
	m_mainLayout = new QHBoxLayout(this);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);
	m_mainLayout->setSpacing(0);

	// 创建左侧导航栏
	createNavigationBar();

	// 创建右侧内容区域
	createContentArea();

	// 设置比例 1:3
	m_mainLayout->setStretch(0, 1);
	m_mainLayout->setStretch(1, 3);
}

void SettingsDialog::createNavigationBar()
{
	m_navigationBar = new QWidget(this);
	m_navigationBar->setFixedWidth(250);
	m_navigationBar->setStyleSheet("QWidget { background-color: #2a2a2a; border-right: 1px solid #333333; }");

	m_navLayout = new QVBoxLayout(m_navigationBar);
	m_navLayout->setContentsMargins(0, 0, 0, 0);
	m_navLayout->setSpacing(0);

	// 应用标题
	QLabel* appTitle = new QLabel("AI桌面助手", m_navigationBar);
	appTitle->setStyleSheet("color: #ffffff; font-size: 18px; font-weight: bold; padding: 20px; background-color: #1a1a1a; border-bottom: 1px solid #333333;");
	appTitle->setAlignment(Qt::AlignCenter);
	m_navLayout->addWidget(appTitle);

	// 导航项
	QString appDir = QApplication::applicationDirPath();
	QString iconDir = appDir + "/../../../images/icons/";

	NavItemWidget* excludedAppsNav = new NavItemWidget(iconDir + "ban.svg", "不监控应用", m_navigationBar);
	NavItemWidget* recordingNav = new NavItemWidget(iconDir + "data-download.svg", "录制回想", m_navigationBar);

	m_navItems.append(excludedAppsNav);
	m_navItems.append(recordingNav);

	m_navLayout->addWidget(excludedAppsNav);
	m_navLayout->addWidget(recordingNav);

	// 底部用户信息区域
	QWidget* userInfo = new QWidget(m_navigationBar);
	userInfo->setFixedHeight(60);
	userInfo->setStyleSheet("QWidget { background-color: #1a1a1a; border-top: 1px solid #333333; }");

	QHBoxLayout* userLayout = new QHBoxLayout(userInfo);
	userLayout->setContentsMargins(20, 10, 20, 10);

	QLabel* userLabel = new QLabel("用户", userInfo);
	userLabel->setStyleSheet("color: #cccccc; font-size: 14px;");

	QPushButton* githubBtn = new QPushButton("GitHub", userInfo);
	githubBtn->setStyleSheet("QPushButton { background-color: transparent; color: #0064ff; border: none; padding: 5px 10px; font-size: 12px; } QPushButton:hover { color: #ffffff; }");

	userLayout->addWidget(userLabel);
	userLayout->addStretch();
	userLayout->addWidget(githubBtn);

	m_navLayout->addWidget(userInfo);

	m_mainLayout->addWidget(m_navigationBar);

	// 连接导航项信号
	for (int i = 0; i < m_navItems.size(); ++i) {
		connect(m_navItems[i], &NavItemWidget::clicked, this, &SettingsDialog::onNavItemClicked);
	}

	// 设置默认选中第一项
	if (!m_navItems.isEmpty()) {
		m_navItems[0]->setSelected(true);
	}
}

void SettingsDialog::createContentArea()
{
	m_contentArea = new QWidget(this);
	m_contentArea->setStyleSheet("QWidget { background-color: #1a1a1a; }");

	QVBoxLayout* contentLayout = new QVBoxLayout(m_contentArea);
	contentLayout->setContentsMargins(0, 0, 0, 0);
	contentLayout->setSpacing(0);

	// 自定义标题栏
	m_titleBar = new QWidget(m_contentArea);
	m_titleBar->setFixedHeight(50);
	m_titleBar->setStyleSheet("QWidget { background-color: #1a1a1a; border-bottom: 1px solid #333333; }");

	QHBoxLayout* titleLayout = new QHBoxLayout(m_titleBar);
	titleLayout->setContentsMargins(20, 0, 20, 0);

	m_titleLabel = new QLabel("设置", m_titleBar);
	m_titleLabel->setStyleSheet("color: #ffffff; font-size: 18px; font-weight: bold;");

	m_minimizeBtn = new QPushButton("—", m_titleBar);
	m_minimizeBtn->setFixedSize(30, 30);
	m_minimizeBtn->setStyleSheet("QPushButton { background-color: #404040; color: #ffffff; border: none; border-radius: 15px; font-size: 16px; font-weight: bold; } QPushButton:hover { background-color: #505050; }");

	m_closeBtn = new QPushButton("×", m_titleBar);
	m_closeBtn->setFixedSize(30, 30);
	m_closeBtn->setStyleSheet("QPushButton { background-color: #ff4444; color: #ffffff; border: none; border-radius: 15px; font-size: 16px; font-weight: bold; } QPushButton:hover { background-color: #ff6666; }");

	titleLayout->addWidget(m_titleLabel);
	titleLayout->addStretch();
	titleLayout->addWidget(m_minimizeBtn);
	titleLayout->addWidget(m_closeBtn);

	contentLayout->addWidget(m_titleBar);

	// 内容堆叠窗口
	m_contentStack = new QStackedWidget(m_contentArea);
	m_contentStack->setStyleSheet("QStackedWidget { background-color: #1a1a1a; }");

	// 初始化页面
	initializeExcludedAppsPage();
	initializeRecordingPage();

	m_contentStack->addWidget(m_excludedAppsPage);
	m_contentStack->addWidget(m_recordingPage);

	contentLayout->addWidget(m_contentStack);

	m_mainLayout->addWidget(m_contentArea);

	connect(m_closeBtn, &QPushButton::clicked, this, &SettingsDialog::close);
	connect(m_minimizeBtn, &QPushButton::clicked, this, &SettingsDialog::showMinimized);
}

void SettingsDialog::initializeExcludedAppsPage()
{
	m_excludedAppsPage = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout(m_excludedAppsPage);
	layout->setContentsMargins(30, 30, 30, 30);
	layout->setSpacing(20);

	// 页面标题
	QLabel* titleLabel = new QLabel("不监控应用名单", m_excludedAppsPage);
	titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff;");
	layout->addWidget(titleLabel);

	// 说明文字
	QLabel* descLabel = new QLabel("添加不需要监控的应用，这些应用切换时不会触发截图和记录", m_excludedAppsPage);
	descLabel->setStyleSheet("color: #cccccc; font-size: 14px; margin-bottom: 20px;");
	descLabel->setWordWrap(true);
	layout->addWidget(descLabel);

	// 输入区域
	QHBoxLayout* inputLayout = new QHBoxLayout();
	inputLayout->setSpacing(15);

	m_excludedAppInput = new QLineEdit(m_excludedAppsPage);
	m_excludedAppInput->setPlaceholderText("输入应用名称（如：notepad.exe）");
	m_excludedAppInput->setMinimumHeight(45);

	m_addExcludedAppBtn = new QPushButton("添加", m_excludedAppsPage);
	m_addExcludedAppBtn->setFixedSize(100, 45);

	inputLayout->addWidget(m_excludedAppInput);
	inputLayout->addWidget(m_addExcludedAppBtn);
	layout->addLayout(inputLayout);

	// 应用列表标题
	QLabel* listTitle = new QLabel("当前不监控的应用：", m_excludedAppsPage);
	listTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #ffffff; margin-top: 20px;");
	layout->addWidget(listTitle);

	// 应用列表
	m_excludedAppsList = new QListWidget(m_excludedAppsPage);
	m_excludedAppsList->setMinimumHeight(300);
	layout->addWidget(m_excludedAppsList);

	// 按钮区域
	QHBoxLayout* buttonLayout = new QHBoxLayout();
	m_removeExcludedAppBtn = new QPushButton("移除选中", m_excludedAppsPage);
	m_removeExcludedAppBtn->setFixedSize(120, 45);
	buttonLayout->addWidget(m_removeExcludedAppBtn);
	buttonLayout->addStretch();
	layout->addLayout(buttonLayout);

	connect(m_addExcludedAppBtn, &QPushButton::clicked, this, &SettingsDialog::addExcludedApp);
	connect(m_removeExcludedAppBtn, &QPushButton::clicked, this, &SettingsDialog::removeExcludedApp);
	connect(m_excludedAppInput, &QLineEdit::returnPressed, this, &SettingsDialog::addExcludedApp);
}

void SettingsDialog::initializeRecordingPage()
{
	m_recordingPage = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout(m_recordingPage);
	layout->setContentsMargins(30, 30, 30, 30);
	layout->setSpacing(20);

	// 页面标题
	QLabel* titleLabel = new QLabel("录制回想", m_recordingPage);
	titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff;");
	layout->addWidget(titleLabel);

	// 说明文字
	QLabel* descLabel = new QLabel("查看历史应用记录，通过时间滑块浏览不同时刻的应用状态", m_recordingPage);
	descLabel->setStyleSheet("color: #cccccc; font-size: 14px; margin-bottom: 20px;");
	descLabel->setWordWrap(true);
	layout->addWidget(descLabel);

	// 时间控制区域
	QHBoxLayout* timeLayout = new QHBoxLayout();
	timeLayout->setSpacing(15);

	QLabel* timeTitle = new QLabel("时间轴:", m_recordingPage);
	timeTitle->setStyleSheet("color: #ffffff; font-size: 14px; font-weight: bold;");
	timeTitle->setFixedWidth(60);

	m_timeSlider = new QSlider(Qt::Horizontal, m_recordingPage);
	m_timeSlider->setEnabled(false);

	m_timeLabel = new QLabel("暂无记录", m_recordingPage);
	m_timeLabel->setStyleSheet("color: #0064ff; font-size: 14px; font-weight: bold;");
	m_timeLabel->setFixedWidth(120);

	timeLayout->addWidget(timeTitle);
	timeLayout->addWidget(m_timeSlider);
	timeLayout->addWidget(m_timeLabel);
	layout->addLayout(timeLayout);

	// 主要内容区域
	QHBoxLayout* contentLayout = new QHBoxLayout();
	contentLayout->setSpacing(20);

	// 左侧应用记录列表
	QVBoxLayout* listLayout = new QVBoxLayout();
	QLabel* listTitle = new QLabel("应用记录列表", m_recordingPage);
	listTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #ffffff;");
	m_appRecordsList = new QListWidget(m_recordingPage);
	m_appRecordsList->setMinimumWidth(300);
	m_appRecordsList->setMinimumHeight(300);

	listLayout->addWidget(listTitle);
	listLayout->addWidget(m_appRecordsList);
	contentLayout->addLayout(listLayout);

	// 右侧详情区域
	QVBoxLayout* detailLayout = new QVBoxLayout();
	QLabel* detailTitle = new QLabel("详细信息", m_recordingPage);
	detailTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #ffffff;");

	m_screenshotLabel = new QLabel(m_recordingPage);
	m_screenshotLabel->setFixedSize(350, 250);
	m_screenshotLabel->setStyleSheet("border: 2px solid #333333; background-color: #2a2a2a; border-radius: 8px; color: #cccccc;");
	m_screenshotLabel->setAlignment(Qt::AlignCenter);
	m_screenshotLabel->setText("暂无截图");

	m_appInfoLabel = new QLabel(m_recordingPage);
	m_appInfoLabel->setStyleSheet("color: #cccccc; font-size: 14px; margin-top: 10px;");
	m_appInfoLabel->setWordWrap(true);
	m_appInfoLabel->setText("请选择一个应用记录查看详情");

	detailLayout->addWidget(detailTitle);
	detailLayout->addWidget(m_screenshotLabel);
	detailLayout->addWidget(m_appInfoLabel);
	detailLayout->addStretch();
	contentLayout->addLayout(detailLayout);

	layout->addLayout(contentLayout);

	connect(m_timeSlider, &QSlider::valueChanged, this, &SettingsDialog::onTimeSliderChanged);
	connect(m_appRecordsList, &QListWidget::currentRowChanged, this, &SettingsDialog::onAppRecordSelected);
}

void SettingsDialog::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && m_titleBar->geometry().contains(event->pos())) {
		m_isDragging = true;
		m_dragStartPos = event->globalPos() - frameGeometry().topLeft();
		event->accept();
	}
}

void SettingsDialog::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton && m_isDragging) {
		move(event->globalPos() - m_dragStartPos);
		event->accept();
	}
}

void SettingsDialog::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		m_isDragging = false;
	}
}

void SettingsDialog::onNavItemClicked()
{
	NavItemWidget* sender = qobject_cast<NavItemWidget*>(this->sender());
	if (sender) {
		int index = m_navItems.indexOf(sender);
		if (index >= 0) {
			switchToPage(index);
		}
	}
}

void SettingsDialog::switchToPage(int index)
{
	if (index >= 0 && index < m_navItems.size() && index < m_contentStack->count()) {
		// 更新导航项选中状态
		for (int i = 0; i < m_navItems.size(); ++i) {
			m_navItems[i]->setSelected(i == index);
		}

		// 切换页面
		m_contentStack->setCurrentIndex(index);
		m_currentNavIndex = index;

		// 更新标题栏标题
		QString title = (index == 0) ? "不监控应用" : "录制回想";
		m_titleLabel->setText(title);
	}
}

QStringList SettingsDialog::getExcludedApps() const
{
	return m_excludedApps;
}

void SettingsDialog::setExcludedApps(const QStringList& apps)
{
	m_excludedApps = apps;
	m_excludedAppsList->clear();
	for (const QString& app : apps) {
		m_excludedAppsList->addItem(app);
	}
}

void SettingsDialog::addAppRecord(const AppRecord& record)
{
	m_appRecords.append(record);
	updateAppRecordsDisplay();
}

void SettingsDialog::clearAppRecords()
{
	m_appRecords.clear();
	updateAppRecordsDisplay();
}

void SettingsDialog::addExcludedApp()
{
	QString appName = m_excludedAppInput->text().trimmed();
	if (appName.isEmpty()) return;

	if (!m_excludedApps.contains(appName)) {
		m_excludedApps.append(appName);
		m_excludedAppsList->addItem(appName);
		m_excludedAppInput->clear();
		emit excludedAppsChanged(m_excludedApps);
	}
}

void SettingsDialog::removeExcludedApp()
{
	int currentRow = m_excludedAppsList->currentRow();
	if (currentRow >= 0) {
		m_excludedApps.removeAt(currentRow);
		delete m_excludedAppsList->takeItem(currentRow);
		emit excludedAppsChanged(m_excludedApps);
	}
}

void SettingsDialog::onAppRecordSelected(int index)
{
	if (index >= 0 && index < m_appRecords.size()) {
		const AppRecord& record = m_appRecords[index];
		m_timeSlider->setValue(index);

		if (!record.screenshot.isNull()) {
			m_screenshotLabel->setPixmap(record.screenshot.scaled(350, 250, Qt::KeepAspectRatio));
		}

		QString info = QString("应用: %1\n时间: %2\n路径: %3")
			.arg(record.appName)
			.arg(record.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
			.arg(record.appPath);
		m_appInfoLabel->setText(info);
	}
}

void SettingsDialog::onTimeSliderChanged(int value)
{
	if (value >= 0 && value < m_appRecords.size()) {
		m_appRecordsList->setCurrentRow(value);
		m_timeLabel->setText(m_appRecords[value].timestamp.toString("hh:mm:ss"));
	}
}

void SettingsDialog::updateAppRecordsDisplay()
{
	m_appRecordsList->clear();
	if (m_appRecords.isEmpty()) {
		m_timeSlider->setEnabled(false);
		return;
	}

	m_timeSlider->setEnabled(true);
	m_timeSlider->setRange(0, m_appRecords.size() - 1);

	for (const AppRecord& record : m_appRecords) {
		QString itemText = QString("%1 - %2").arg(record.timestamp.toString("hh:mm:ss")).arg(record.appName);
		m_appRecordsList->addItem(itemText);
	}
}