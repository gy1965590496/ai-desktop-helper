#include "recordingwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

RecordingWidget::RecordingWidget(QWidget* parent)
	: QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(30, 30, 30, 30);
	layout->setSpacing(20);

	// 标题
	QLabel* titleLabel = new QLabel("录制回想", this);
	titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff;");
	layout->addWidget(titleLabel);

	QLabel* descLabel = new QLabel("查看历史应用记录，通过时间滑块浏览不同时刻的应用状态", this);
	descLabel->setStyleSheet("color: #cccccc; font-size: 14px;");
	descLabel->setWordWrap(true);
	layout->addWidget(descLabel);

	// 时间滑块区域
	QHBoxLayout* timeLayout = new QHBoxLayout();
	QLabel* timeTitle = new QLabel("时间轴:", this);
	timeTitle->setStyleSheet("color: #ffffff; font-size: 14px; font-weight: bold;");
	timeTitle->setFixedWidth(60);

	m_timeSlider = new QSlider(Qt::Horizontal, this);
	m_timeSlider->setEnabled(false);

	m_timeLabel = new QLabel("暂无记录", this);
	m_timeLabel->setStyleSheet("color: #0064ff; font-size: 14px; font-weight: bold;");
	m_timeLabel->setFixedWidth(120);

	timeLayout->addWidget(timeTitle);
	timeLayout->addWidget(m_timeSlider);
	timeLayout->addWidget(m_timeLabel);
	layout->addLayout(timeLayout);

	// 主体内容区域
	QHBoxLayout* contentLayout = new QHBoxLayout();
	contentLayout->setSpacing(20);

	// 应用记录列表
	QVBoxLayout* listLayout = new QVBoxLayout();
	QLabel* listTitle = new QLabel("应用记录列表", this);
	listTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #ffffff;");
	m_appRecordsList = new QListWidget(this);
	m_appRecordsList->setMinimumWidth(300);
	m_appRecordsList->setMinimumHeight(300);

	listLayout->addWidget(listTitle);
	listLayout->addWidget(m_appRecordsList);
	contentLayout->addLayout(listLayout);

	// 右侧详情
	QVBoxLayout* detailLayout = new QVBoxLayout();
	QLabel* detailTitle = new QLabel("详细信息", this);
	detailTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #ffffff;");

	m_screenshotLabel = new QLabel(this);
	m_screenshotLabel->setFixedSize(350, 250);
	m_screenshotLabel->setStyleSheet("border: 2px solid #333333; background-color: #2a2a2a; border-radius: 8px; color: #cccccc;");
	m_screenshotLabel->setAlignment(Qt::AlignCenter);
	m_screenshotLabel->setText("暂无截图");

	m_appInfoLabel = new QLabel(this);
	m_appInfoLabel->setStyleSheet("color: #cccccc; font-size: 14px; margin-top: 10px;");
	m_appInfoLabel->setWordWrap(true);
	m_appInfoLabel->setText("请选择一个应用记录查看详情");

	detailLayout->addWidget(detailTitle);
	detailLayout->addWidget(m_screenshotLabel);
	detailLayout->addWidget(m_appInfoLabel);
	detailLayout->addStretch();
	contentLayout->addLayout(detailLayout);

	layout->addLayout(contentLayout);

	connect(m_timeSlider, &QSlider::valueChanged, this, &RecordingWidget::onTimeSliderChanged);
	connect(m_appRecordsList, &QListWidget::currentRowChanged, this, &RecordingWidget::onAppRecordSelected);
}

void RecordingWidget::addAppRecord(const AppRecord& record)
{
	m_appRecords.append(record);
	updateAppRecordsDisplay();
}

void RecordingWidget::clearAppRecords()
{
	m_appRecords.clear();
	updateAppRecordsDisplay();
}

void RecordingWidget::syncAppRecords(const QList<AppRecord>& records)
{
	m_appRecords.clear();
	for (const AppRecord& record : records) {
		m_appRecords.append(record);
	}
	updateAppRecordsDisplay();
	qDebug() << "同步应用记录，数量:" << records.size();
}

void RecordingWidget::onTimeSliderChanged(int value)
{
	if (value >= 0 && value < m_appRecords.size()) {
		m_appRecordsList->setCurrentRow(value);
		const AppRecord& record = m_appRecords[value];
		m_timeLabel->setText(record.timestamp.toString("hh:mm:ss"));
		
		// 更新详情显示
		if (!record.screenshot.isNull()) {
			m_screenshotLabel->setPixmap(record.screenshot.scaled(350, 250, Qt::KeepAspectRatio));
		} else {
			m_screenshotLabel->setText("暂无截图");
			m_screenshotLabel->setPixmap(QPixmap());
		}

		QString info = QString("应用: %1\n时间: %2\n路径: %3\n窗口标题: %4")
			.arg(record.appName)
			.arg(record.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
			.arg(record.appPath)
			.arg(record.windowTitle);
		m_appInfoLabel->setText(info);
	}
}

void RecordingWidget::onAppRecordSelected(int index)
{
	if (index >= 0 && index < m_appRecords.size()) {
		const AppRecord& record = m_appRecords[index];
		m_timeSlider->setValue(index);

		if (!record.screenshot.isNull()) {
			m_screenshotLabel->setPixmap(record.screenshot.scaled(350, 250, Qt::KeepAspectRatio));
		} else {
			m_screenshotLabel->setText("暂无截图");
			m_screenshotLabel->setPixmap(QPixmap());
		}

		QString info = QString("应用: %1\n时间: %2\n路径: %3")
			.arg(record.appName)
			.arg(record.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
			.arg(record.appPath);
		m_appInfoLabel->setText(info);

		emit recordSelected(record);
	}
}

void RecordingWidget::updateAppRecordsDisplay()
{
	m_appRecordsList->clear();

	if (m_appRecords.isEmpty()) {
		m_timeSlider->setEnabled(false);
		m_timeLabel->setText("暂无记录");
		return;
	}

	m_timeSlider->setEnabled(true);
	m_timeSlider->setRange(0, m_appRecords.size() - 1);

	// 显示时间范围信息
	QDateTime firstTime = m_appRecords.first().timestamp;
	QDateTime lastTime = m_appRecords.last().timestamp;
	QString timeRange = QString("%1 - %2")
		.arg(firstTime.toString("hh:mm:ss"))
		.arg(lastTime.toString("hh:mm:ss"));
	m_timeLabel->setText(timeRange);

	for (const AppRecord& record : m_appRecords) {
		QString itemText = QString("%1 - %2")
			.arg(record.timestamp.toString("hh:mm:ss"))
			.arg(record.appName);
		m_appRecordsList->addItem(itemText);
	}
	
	qDebug() << "更新应用记录显示，记录数量:" << m_appRecords.size();
}
