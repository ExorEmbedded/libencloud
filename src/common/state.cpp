#include <QString>
#include <encloud/State>

namespace libencloud 
{

QString stateToString (State state)
{
    switch (state)
    {
        case StateIdle:
            return "Idle";
        case StateError:
            return "Error";
        case StateSetup:
            return "Setting Up";
        case StateConnect:
            return "Connecting";
        case StateCloud:
            return "Connected";
    }
    return "";
}

}  // namespace libencloud
