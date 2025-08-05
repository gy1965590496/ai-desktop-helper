#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QMouseEvent>
#include <QEvent>
#include <QPixmap>
#include <QList>
#include "common.h"

// NavItemWidget类定义
class NavItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NavItemWidget(const QString& iconPath, const QString& text, QWidget* parent = nullptr);
    void setSelected(bool selected);

signals:
    void clicked();

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QLabel* _iconLabel;
    QLabel* _textLabel;
    QString _iconPath;
    QString _text;
    bool _hovered;
    bool _selected;
    void updateStyle();
};

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    QStringList getExcludedApps() const;
    void setExcludedApps(const QStringList &apps);
    void addAppRecord(const AppRecord &record);
    void clearAppRecords();

signals:
    void excludedAppsChanged(const QStringList &apps);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onNavItemClicked();
    void addExcludedApp();
    void removeExcludedApp();
    void onTimeSliderChanged(int value);
    void onAppRecordSelected(int index);

private:
    void initializeUI();
    void createNavigationBar();
    void createContentArea();
    void initializeExcludedAppsPage();
    void initializeRecordingPage();
    void updateAppRecordsDisplay();
    void switchToPage(int index);

private:
    bool m_isDragging;
    QPoint m_dragStartPos;
    QHBoxLayout *m_mainLayout;
    QWidget *m_navigationBar;
    QVBoxLayout *m_navLayout;
    QList<NavItemWidget*> m_navItems;
    int m_currentNavIndex;
    QWidget *m_contentArea;
    QStackedWidget *m_contentStack;
    QWidget *m_excludedAppsPage;
    QLineEdit *m_excludedAppInput;
    QPushButton *m_addExcludedAppBtn;
    QListWidget *m_excludedAppsList;
    QPushButton *m_removeExcludedAppBtn;
    QStringList m_excludedApps;
    QWidget *m_recordingPage;
    QSlider *m_timeSlider;
    QLabel *m_timeLabel;
    QListWidget *m_appRecordsList;
    QLabel *m_screenshotLabel;
    QLabel *m_appInfoLabel;
    QList<AppRecord> m_appRecords;
    QWidget *m_titleBar;
    QLabel *m_titleLabel;
    QPushButton *m_closeBtn;
    QPushButton *m_minimizeBtn;
};

#endif // SETTINGSDIALOG_H

 