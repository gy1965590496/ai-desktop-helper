#pragma once

#include <QWidget>
#include <QStringList>

class QLineEdit;
class QListWidget;
class QPushButton;

class AppFilterWidget : public QWidget {
    Q_OBJECT
public:
    explicit AppFilterWidget(QWidget* parent = nullptr);

    // 获取当前过滤的应用列表
    QStringList getFilteredApps() const;
    
    // 设置过滤的应用列表
    void setFilteredApps(const QStringList& apps);

signals:
    // 应用过滤列表变化信号
    void filteredAppsChanged(const QStringList& apps);

private slots:
    void onAddAppClicked();
    void onRemoveAppClicked();
    void onFilterTextChanged(const QString& text);

private:
    void refreshList();
    void emitFilteredAppsChanged();
    void addDefaultSystemApps();

    QLineEdit* _filterEdit;
    QListWidget* _appListWidget;
    QPushButton* _addButton;
    QPushButton* _removeButton;

    QStringList _allAppPaths;  // 全部应用路径
};
