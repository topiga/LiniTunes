#ifndef NETMUXD_MANAGER_H
#define NETMUXD_MANAGER_H

#include <QObject>
#include <QProcess>

class NetmuxdManager : public QObject
{
public:
    explicit NetmuxdManager(QObject *parent = nullptr);
    ~NetmuxdManager() override;

    void ensureRunning();
    void stop();

private:
    QString bundledNetmuxdPath() const;
    QString lockdownDir() const;
    bool muxResponds(const QString &address) const;
    void logOutput(QProcess::ProcessChannel channel);

    QProcess *m_process = nullptr;
};

#endif // NETMUXD_MANAGER_H
