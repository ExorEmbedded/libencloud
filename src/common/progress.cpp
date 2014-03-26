#include <encloud/Progress>
#include <common/common.h>
#include <common/config.h>

namespace libencloud 
{

Progress::Progress ()
    : _isValid(false)
    , _step(0)
    , _total(0)
{
}

Progress::Progress (const QString &desc, int step, int total)
    : _isValid(false)
    , _desc(desc)
    , _step(step)
    , _total(total)
{
    LIBENCLOUD_ERR_IF (desc == "");
    LIBENCLOUD_ERR_IF (step <= 0);
    LIBENCLOUD_ERR_IF (total <= 0);

    LIBENCLOUD_DBG(QString::number(step) << "/" << QString::number(total) << " " << desc);

    _isValid = true;

err:
    return;
}

bool Progress::isValid () const   { return _isValid; }

bool Progress::operator == (const Progress &p) const
{
    return (_desc == p._desc &&
            _step == p._step &&
            _total == p._total);
}

bool Progress::operator != (const Progress &p) const
{
    return !(p == *this);
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
