#include <encloud/Manager/ProcessManager>
#include <common/common.h>
#include <common/config.h>

namespace libencloud
{

//
// public methods
// 

ProcessManager::ProcessManager ()
{
    LIBENCLOUD_TRACE;
}

ProcessManager::~ProcessManager ()
{
    LIBENCLOUD_DBG("[ProcessManager] count: " << QString::number(_processes.count()));

    qDeleteAll (_processes);
}

QProcess *ProcessManager::start (const QString &path, const QString &args)
{
    return startEx(false, path, args);
}

QProcess *ProcessManager::startDetached (const QString &path, const QString &args)
{
    return startEx(true, path, args);
}

//
// private slots
//

void ProcessManager::processStateChanged (Process::State state)
{
    Process *process = static_cast<Process *>(QObject::sender());

    if (state == Process::StateZombie)
    {
        removeProcess(process);
    }
}

//
// private methods
//

void ProcessManager::removeProcess (Process *process)
{
    QMutableListIterator<Process *> it(_processes);

    while (it.hasNext())
    {        
        if (it.next() == process)
        {
            delete process;
            it.remove();
            return;
        }
    }
}

QProcess *ProcessManager::startEx (bool detach, const QString &path, const QString &args)
{
    QProcess *qProcess = NULL;
    Process *process = NULL;
        
    LIBENCLOUD_DBG("[ProcessManager] Executing - detach: " << detach << ", path: " << path << ", args: " << args);

    process = new Process(path, args);
    LIBENCLOUD_ERR_IF (process == NULL);

    if (detach)
        qProcess = process->startDetached();
    else
        qProcess = process->start();
    LIBENCLOUD_ERR_IF (process == NULL);

    connect(process, SIGNAL(stateChanged(Process::State)),
                this, SLOT(processStateChanged(Process::State)));

    _processes.append(process);

    return qProcess;
err:
    LIBENCLOUD_DELETE(process);
    return NULL;
}

}  // namespace libencloud
