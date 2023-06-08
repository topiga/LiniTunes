#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFileInfo>
#include <QIcon>

#include <QLocale>
#include <QTranslator>
#include <idevice.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

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
    QFileInfo fi(app.applicationDirPath() + "/../share/pixmaps/linitunes.png");
    QGuiApplication::setWindowIcon(QIcon(fi.absoluteFilePath()));

    iDevice *Device = new iDevice();

    const QUrl url(u"qrc:/LiniTunes/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    delete Device;

    return app.exec();
}
