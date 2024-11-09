#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QQmlContext>

// Translation purposes (TODO)
#include <QLocale>
#include <QTranslator>

// Device purposes
#include <idevice.h>
#include <idevicewatcher.h>

// Theme
#include <thememanager.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setWindowIcon(QIcon(":/images/linitunes.png"));

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

    const QUrl url(u"qrc:/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    // We need to connect the dots right after the QML,
    // or the devices might not be recognized.
    if (DeviceWatcher->begin() != 0) {
        app.quit();
    }

    return app.exec();
    delete DeviceWatcher;
}
