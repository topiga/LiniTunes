#ifndef SOFTWARE_UPDATE_MANAGER_H
#define SOFTWARE_UPDATE_MANAGER_H

#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QVariantList>

class QNetworkReply;

struct IpswEntry {
    QString productType;
    QString version;
    QString build;
    QString url;
    QString sha1;
    QString sha256;
    QString md5;
    qint64 size = 0;
    bool signedRestore = true;

    QVariantMap toMap() const;
    QString key() const;
};

class SoftwareUpdateManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool downloading READ downloading NOTIFY changed)
    Q_PROPERTY(double download_progress READ downloadProgress NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QVariantList update_candidates READ updateCandidates NOTIFY changed)
    Q_PROPERTY(QVariantList restore_candidates READ restoreCandidates NOTIFY changed)
    Q_PROPERTY(QString downloaded_path READ downloadedPath NOTIFY changed)

public:
    explicit SoftwareUpdateManager(QObject *parent = nullptr);

    bool busy() const { return m_busy; }
    bool downloading() const { return m_downloading; }
    double downloadProgress() const { return m_downloadProgress; }
    QString status() const { return m_status; }
    QString error() const { return m_error; }
    QVariantList updateCandidates() const;
    QVariantList restoreCandidates() const;
    QString downloadedPath() const { return m_downloadedPath; }

    Q_INVOKABLE void check(const QString &productType, const QString &currentVersion, const QString &currentBuild,
                           bool silent = false);
    Q_INVOKABLE void downloadUpdate(int index = 0);
    Q_INVOKABLE void downloadRestore(int index = 0);
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void reset();

signals:
    void changed();

private:
    void setStatus(const QString &status, const QString &error = QString());
    void finishCheck(QVector<IpswEntry> entries);
    void fetchNextMetadata();
    void applyMetadata(QNetworkReply *reply, IpswEntry *entry);
    void applyEntryToCandidates(const IpswEntry &entry);
    void startRangeMetadata(const IpswEntry &entry);
    void downloadFromList(const QVector<IpswEntry> &entries, int index);
    void startDownload(const IpswEntry &entry, bool retry = false);
    void retryDownload(const QString &message);
    void stopDownloadReply();
    void finishDownload(bool cancelled = false);
    void failDownload(const QString &message);
    bool writeDownloadChunk(const QByteArray &data);
    void restartIdleTimer();

    QVector<IpswEntry> parseCatalog(const QByteArray &data, const QString &productType) const;
    QVariantList toVariantList(const QVector<IpswEntry> &entries) const;
    QString cacheDir() const;
    QString indexPath() const;
    QString finalPath(const IpswEntry &entry) const;
    QString partPath(const IpswEntry &entry) const;
    QJsonArray readIndex() const;
    void writeIndex(const QJsonArray &items) const;
    QString verifiedPathFor(const IpswEntry &entry) const;
    void rememberVerified(const IpswEntry &entry, const QString &path);

    QNetworkRequest requestFor(const QString &url) const;
    bool isSafeAppleUrl(const QString &url) const;
    bool verifyHash(const IpswEntry &entry, const QByteArray &sha256, const QByteArray &sha1) const;

    QNetworkAccessManager m_net;
    QVector<IpswEntry> m_updateCandidates;
    QVector<IpswEntry> m_restoreCandidates;
    QVector<IpswEntry> m_metadataQueue;
    QString m_currentVersion;
    QString m_currentBuild;
    QString m_status = QStringLiteral("idle");
    QString m_error;
    QString m_downloadedPath;
    bool m_busy = false;
    bool m_downloading = false;
    bool m_cancelled = false;
    bool m_silentCheck = false;
    double m_downloadProgress = 0;

    IpswEntry m_downloadEntry;
    QPointer<QNetworkReply> m_downloadReply;
    QFile m_downloadFile;
    QCryptographicHash m_sha256;
    QCryptographicHash m_sha1;
    qint64 m_downloadedBytes = 0;
    int m_generation = 0;
    int m_downloadRetries = 0;
    QTimer m_idleTimer;
};

#endif // SOFTWARE_UPDATE_MANAGER_H
