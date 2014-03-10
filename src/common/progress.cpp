#include <encloud/Progress>
#include <common/common.h>
#include <common/config.h>

namespace libencloud 
{

Progress::Progress ()
    : _step(0)
    , _total(0)
{
}

Progress::Progress (const QString &desc, int step, int total)
    : _desc(desc)
    , _step(step)
    , _total(total)
{
    LIBENCLOUD_DBG(QString::number(step) << "/" << QString::number(total) << " " << desc);
}

bool Progress::operator== (const Progress &p) const
{
    return (_desc == p._desc &&
            _step == p._step &&
            _total == p._total);
}

QString Progress::getDesc () const
{
    return _desc;
}

void Progress::setDesc (const QString &desc)
{
    _desc = desc;
}

int Progress::getStep () const
{
    return _step;
}

void Progress::setStep (int step)
{
    _step = step;
}

int Progress::getTotal () const
{
    return _total;
}

void Progress::setTotal (int total)
{
    _total = total;
}

}  // namespace libencloud
