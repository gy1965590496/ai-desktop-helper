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
#include "../common.h"
#include "recordingwidget.h"
#include "appFilterWidget.h"
#include "settingsnavigationbar.h"

// 前向声明
class ScreenMonitor;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    
    // 设置 ScreenMonitor 引用
    void setScreenMonitor(ScreenMonitor* monitor);
    
    // 记录管理公共方法
    void addAppRecord(const AppRecord& record);
    void clearAppRecords();

signals:
    // 不监控应用名单变化信号
    void excludedAppsChanged(const QStringList &apps);
    
    // 应用过滤列表变化信号
    void appFiltersChanged(const QStringList &filteredApps);

protected:
    // 鼠标事件处理，实现窗口拖动
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    // 左侧导航栏选择变化
    void onNavigationItemChanged(int index);
    
    // 应用过滤列表变化
    void onAppFiltersChanged(const QStringList &filteredApps);
    
    // 记录管理
    void onAppRecordAdded(const AppRecord &record);
    void onAppRecordsCleared();

private:
    // 初始化UI
    void initializeUI();
    
    // 创建左侧导航栏
    void createNavigationBar();
    
    // 创建右侧内容区域
    void createContentArea();
    
    // 初始化不监控应用页面
    void initializeExcludedAppsPage();
    
    // 初始化录制回想页面
    void initializeRecordingPage();
    
    // 更新应用记录显示
    void updateAppRecordsDisplay();

private:
    // 窗口拖动相关
    bool m_isDragging;
    QPoint m_dragStartPos;
    
    // 主布局
    QHBoxLayout *m_mainLayout;

    SettingsNavigationBar* m_navigationBar;
    RecordingWidget* m_recordingWidget;
    AppFilterWidget* m_appFilterWidget;
    
    // 左侧导航栏
    QListWidget *m_navigationList;
    
    // 右侧内容区域
    QWidget *m_contentArea;
    QStackedWidget *m_contentStack;
    
    // 不监控应用页面
    QWidget *m_excludedAppsPage;
    QLineEdit *m_excludedAppInput;
    QPushButton *m_addExcludedAppBtn;
    QListWidget *m_excludedAppsList;
    QPushButton *m_removeExcludedAppBtn;
    QStringList m_excludedApps;
    
    // 录制回想页面
    QWidget *m_recordingPage;
    QSlider *m_timeSlider;
    QLabel *m_timeLabel;
    QListWidget *m_appRecordsList;
    QLabel *m_screenshotLabel;
    QLabel *m_appInfoLabel;
    QList<AppRecord> m_appRecords;
    
    // 标题栏
    QWidget *m_titleBar;
    QLabel *m_titleLabel;
    QPushButton *m_closeBtn;
    QPushButton *m_minimizeBtn;
    
    // ScreenMonitor 引用
    ScreenMonitor *m_screenMonitor;
};

#endif // SETTINGSDIALOG_H 