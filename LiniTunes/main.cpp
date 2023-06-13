#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QIcon>

#include <QLocale>
#include <QTranslator>
#include <idevice.h>
#include <idevicewatcher.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setWindowIcon(QIcon(":/images/linitunes.png"));

    iDeviceWatcher *DeviceWatcher = new iDeviceWatcher();

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


    const QUrl url(u"qrc:/LiniTunes/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
    delete DeviceWatcher;
}
