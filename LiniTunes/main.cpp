#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QQmlContext>
#include <QFontDatabase>

// Translation purposes (TODO)
#include <QLocale>
#include <QTranslator>

// Device purposes
#include <idevicewatcher.h>

// Theme
#include <thememanager.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setWindowIcon(QIcon(":/images/linitunes.png"));

    QString appFontFamily;
    const int fontId = QFontDatabase::addApplicationFont(":/ressources/fonts/inter_variable_font.ttf");
    if (fontId != -1) {
        const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            appFontFamily = families.first();
            QFont appFont(appFontFamily);
            app.setFont(appFont);
        }
    }

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "LiniTunes_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    QQmlApplicationEngine engine;

    iDeviceWatcher *DeviceWatcher = new iDeviceWatcher();
    engine.rootContext()->setContextProperty("DeviceWatcher", DeviceWatcher);

    ThemeManager *themeManager = new ThemeManager();
    engine.rootContext()->setContextProperty("ThemeManager", themeManager);
    engine.rootContext()->setContextProperty("AppFontFamily", appFontFamily);

    const QUrl url(u"qrc:/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    // Start device listener after QML is loaded
    DeviceWatcher->start();

    return app.exec();
}