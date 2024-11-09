#include "thememanager.h"

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    // Connect to system theme changes
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, &ThemeManager::onSystemThemeChanged);
}

bool ThemeManager::isDarkMode() const
{
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

void ThemeManager::onSystemThemeChanged()
{
    emit themeChanged();
}
