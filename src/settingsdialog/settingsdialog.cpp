#include "settingsdialog.h"
#include <QDebug>
#include <QApplication>

SettingsDialog::SettingsDialog(QWidget* parent)
	: QDialog(parent)
	, m_isDragging(false)
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
	m_navigationBar = new SettingsNavigationBar(this);
    m_mainLayout->addWidget(m_navigationBar);

    connect(m_navigationBar, &SettingsNavigationBar::navigationItemChanged,
            this, &SettingsDialog::onNavigationItemChanged);
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
	// m_excludedAppsWidget = new ExcludedAppsWidget(m_contentStack);
	m_recordingWidget = new RecordingWidget(m_contentStack);
	m_appFilterWidget = new AppFilterWidget(m_contentStack);

	// m_contentStack->addWidget(m_excludedAppsWidget);
	m_contentStack->addWidget(m_recordingWidget);
	m_contentStack->addWidget(m_appFilterWidget);

	contentLayout->addWidget(m_contentStack);

	m_mainLayout->addWidget(m_contentArea);

	connect(m_closeBtn, &QPushButton::clicked, this, &SettingsDialog::close);
	connect(m_minimizeBtn, &QPushButton::clicked, this, &SettingsDialog::showMinimized);
	
	// 连接 AppFilterWidget 信号
	connect(m_appFilterWidget, &AppFilterWidget::filteredAppsChanged,
	        this, &SettingsDialog::onAppFiltersChanged);
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

void SettingsDialog::onNavigationItemChanged(int index)
{
	if (index >= 0 && index < m_contentStack->count()) {
		m_contentStack->setCurrentIndex(index);

		// 更新标题栏标题
		QString title = (index == 0) ? "录制回想" : "应用过滤";
		m_titleLabel->setText(title);
	}
}

void SettingsDialog::onAppFiltersChanged(const QStringList &filteredApps)
{
	qDebug() << "应用过滤列表已更新:" << filteredApps;
	emit appFiltersChanged(filteredApps);
}


