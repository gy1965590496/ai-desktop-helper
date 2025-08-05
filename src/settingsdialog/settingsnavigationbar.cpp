#include "settingsnavigationbar.h"
#include <QVBoxLayout>
#include <QLabel>

SettingsNavigationBar::SettingsNavigationBar(QWidget* parent)
	: QWidget(parent)
{
	setFixedWidth(250);
	setStyleSheet("QWidget { background-color: #2a2a2a; border-right: 1px solid #333333; }");

	QVBoxLayout* navLayout = new QVBoxLayout(this);
	navLayout->setContentsMargins(0, 0, 0, 0);
	navLayout->setSpacing(0);

	QLabel* appTitle = new QLabel("设置", this);
	appTitle->setStyleSheet("color: #ffffff; font-size: 18px; font-weight: bold; padding: 20px; background-color: #1a1a1a; border-bottom: 1px solid #333333;");
	appTitle->setAlignment(Qt::AlignCenter);
	navLayout->addWidget(appTitle);

	m_navigationList = new QListWidget(this);
	m_navigationList->setStyleSheet(
		"QListWidget { background-color: #2a2a2a; border: none; outline: none; }"
		"QListWidget::item { background-color: transparent; border: none; padding: 15px 20px; margin: 0; border-radius: 0; color: #cccccc; font-size: 14px; }"
		"QListWidget::item:hover { background-color: #404040; color: #ffffff; }"
		"QListWidget::item:selected { background-color: #0064ff; color: #ffffff; }"
	);


	QStringList navItems = { "录制回想", "应用过滤器" };
	for (const QString& item : navItems) {
		m_navigationList->addItem(item);
	}
	m_navigationList->setCurrentRow(0);
	navLayout->addWidget(m_navigationList);

	connect(m_navigationList, &QListWidget::currentRowChanged,
		this, &SettingsNavigationBar::navigationItemChanged);
}
