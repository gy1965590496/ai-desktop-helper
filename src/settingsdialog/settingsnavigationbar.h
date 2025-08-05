#pragma once

#include <QWidget>
#include <QListWidget>

class SettingsNavigationBar : public QWidget {
    Q_OBJECT
public:
    explicit SettingsNavigationBar(QWidget* parent = nullptr);

signals:
    void navigationItemChanged(int index);

private:
    QListWidget* m_navigationList;
};
