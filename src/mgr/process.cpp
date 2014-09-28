#include <QTimer>
#include <encloud/Manager/Process>
#include <common/common.h>
#include <common/config.h>

namespace libencloud
{

//
// public methods
// 

Process::Process (const QString &path, const QString &args)
    : _path(path)
    , _args(args)
    , _process(NULL)
    , _err(NoError)
    , _state(StateIdle)
{
}

Process::~Process ()
{
    //LIBENCLOUD_TRACE;

    stop();
}

QProcess *Process::start ()
{
    _detach = false;

    return startEx();
}

QProcess *Process::startDetached ()
{
    _detach = true;

    return startEx();
}

int Process::stop ()
{
    if (_process)
    {
        _process->blockSignals(true);
#ifdef Q_OS_WIN32
        _process->kill();
#else
        _process->terminate();
#endif
        _process->waitForFinished();
        LIBENCLOUD_DELETE(_process);
    }

    _err = NoError;

    return 0;
}

QString Process::getPath ()
{
    return _path;
}

QString Process::getArgs ()
{
    return _args;
}

bool Process::getDetached ()
{
    return _detach;
}

//
// private slots
//

void Process::processError (QProcess::ProcessError err)
{
    LIBENCLOUD_DBG("[Process] err: " << err << " (" << _process->errorString() << ")") ;

    emit stateChanged((_state = StateIdle));
}

void Process::processStarted ()
{
    emit stateChanged((_state = StateStarted));
}

void Process::processFinished (int exitCode, QProcess::ExitStatus exitStatus)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_EMIT_ERR_MSG_IF (exitCode || exitStatus,
            error(_err = ProcessError),
            "Process exitCode: " + QString::number(exitCode) +
            ", exitStatus: " + QString::number(exitStatus));

err:
    emit stateChanged((_state = StateIdle));
    QTimer::singleShot(LIBENCLOUD_PROCESS_ZOMBIE_TOUT, this, SLOT(processZombie()));
}

void Process::processZombie ()
{
    LIBENCLOUD_DBG("[Process] path: " << _path << ", args: " << _args);

    emit stateChanged((_state = StateZombie));
}

//
// private methods
//

QProcess *Process::startEx ()
{
    QStringList args;

    LIBENCLOUD_EMIT_ERR_MSG_IF (_state != StateIdle ||
                         _process != NULL,
            error(_err = BadStateError),
            "Bad state: " + QString::number(_state));

    _process = new QProcess();
    LIBENCLOUD_EMIT_ERR_IF (_process == NULL,
            error(_err = MemoryError));

    connect(_process, SIGNAL(error(QProcess::ProcessError)), this,
            SLOT(processError(QProcess::ProcessError)));
    connect(_process, SIGNAL(started()), this,
            SLOT(processStarted()));
    connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(processFinished(int, QProcess::ExitStatus)));

    if (_args != "")
        args = _args.split(" ");

    if (_detach)
        _process->startDetached(_path, args);
    else 
        _process->start(_path, args);

    emit stateChanged((_state = StateStarting));

    return _process;

err:
    LIBENCLOUD_DELETE(_process);
    return NULL;
}

}  // namespace libencloud
