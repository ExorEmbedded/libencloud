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
        case StateSetup:
            return "Setting Up";
        case StateConnect:
            return "Connecting";
        case StateCloud:
            return "Connected";
        case StateError:
            return "Error";
    }
    return "";
}

}  // namespace libencloud
