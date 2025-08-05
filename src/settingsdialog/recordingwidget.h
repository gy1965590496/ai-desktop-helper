#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QSlider>
#include <QPixmap>
#include <QDateTime>
#include <QVector>
#include "../common.h"

class RecordingWidget : public QWidget {
	Q_OBJECT
public:
	explicit RecordingWidget(QWidget* parent = nullptr);

	void addAppRecord(const AppRecord& record);
	void clearAppRecords();
	void syncAppRecords(const QList<AppRecord>& records);

signals:
	void recordSelected(const AppRecord& record);

private slots:
	void onTimeSliderChanged(int value);
	void onAppRecordSelected(int index);

private:
	void updateAppRecordsDisplay();

private:
	QListWidget* m_appRecordsList;
	QLabel* m_timeLabel;
	QSlider* m_timeSlider;
	QLabel* m_screenshotLabel;
	QLabel* m_appInfoLabel;

	QVector<AppRecord> m_appRecords;
};
