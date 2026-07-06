#ifndef NETMUXD_MANAGER_H
#define NETMUXD_MANAGER_H

#include <QObject>
#include <QProcess>

class NetmuxdManager : public QObject
{
    Q_OBJECT
public:
    explicit NetmuxdManager(QObject *parent = nullptr);
    ~NetmuxdManager() override;

    void ensureRunning();
    void stop();

    QString status() const { return m_status; }
    QString error() const { return m_error; }

signals:
    void statusChanged();

private:
    QString bundledNetmuxdPath() const;
    QString lockdownDir() const;
    bool muxResponds(const QString &address) const;
    void logOutput(QProcess::ProcessChannel channel);
    void setStatus(const QString &status, const QString &error = QString());

    QProcess *m_process = nullptr;
    QString m_status = QStringLiteral("idle");
    QString m_error;
    bool m_stopping = false;
};

#endif // NETMUXD_MANAGER_H
