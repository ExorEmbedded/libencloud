#ifndef _LIBENCLOUD_PRIV_VPNCLIENT_H_
#define _LIBENCLOUD_PRIV_VPNCLIENT_H_

#include <QProcess>
#include <QTemporaryFile>
#include <QUrl>
#include <common/config.h>

namespace libencloud 
{

class VpnClient : public QObject
{
    Q_OBJECT

public:
    // keep me aligned with errorString()
    typedef enum
    {
        NoError = 0,
        BadParamsError,
        MemoryError,
        BadStateError,
        ConfigError,
        NotExecError,
        ProcessError
    }
    Error;

    // keep me aligned with stateString()
    typedef enum
    {
        StateIdle = 0,
        StateConfigured,
        StateStarting,
        StateStarted,
        // following states controlled by VpnManager
        StateConnecting,
        StateConnectingWaitingInitialResponse,
        StateConnectingAuthenticating,
        StateConnectingDownloadingConfiguration,
        StateConnectingAssigningIP,
        StateConnectingAddingRoutes,
        StateConnected,
        StateExiting,

        StateFirst = StateConfigured,
        StateLast = StateExiting
    }
    State;

    VpnClient (const Config *config);
    ~VpnClient ();

    Error error ();
    static QString errorString (Error err);
    
    State state ();
    static QString stateString (State st);
    void setState (State st);  // for VpnManager and Manager

    static bool checkState (VpnClient::State state);

    QStringList getArgs (void);
    void start (void);
    void stop (void);

signals:
    void stateChanged (VpnClient::State state);
    void sigError (VpnClient::Error err, QString msg = "");

private slots:

#if 0
    void gotLogin (const QString &url, const QString &user, const QString &pass);
    void sbGotConfig (QString sbConfig);
#endif

    void processError (QProcess::ProcessError err);
    void processStarted ();
    void processFinished (int exitCode, QProcess::ExitStatus exitStatus);

private:
    void enableTap ();
    void disableTap ();
    int parseConfig (const QVariantMap &jo);
    int parseCaCert (const QVariantMap &jo);

    QUrl remoteUrl;
    int remotePort;
    const Config *_cfg;
    Error err;
    State st;
    QTemporaryFile configFile;
    QTemporaryFile caCertFile;
    QProcess *process;

};

}  // namespace libencloud

#endif  /* _LIBENCLOUD_VPNCLIENT_H_ */
