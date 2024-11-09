#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QGuiApplication>
#include <QStyleHints>

class ThemeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isDarkMode READ isDarkMode NOTIFY themeChanged)
public:
    explicit ThemeManager(QObject *parent = nullptr);
    bool isDarkMode() const;

signals:
    void themeChanged();

private slots:
    void onSystemThemeChanged();
};

#endif // THEMEMANAGER_H
