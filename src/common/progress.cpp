#include <encloud/Progress>

namespace libencloud 
{

Progress::Progress ()
    : _step(0)
    , _total(0)
{
}

Progress::Progress (QString desc, int step, int total)
    : _desc(desc)
    , _step(step)
    , _total(total)
{
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
