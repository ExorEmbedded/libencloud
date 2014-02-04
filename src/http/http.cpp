#include <encloud/Http>

namespace libencloud {

//
// public methods
//

QString httpStatusString (HttpStatus status)
{
    switch (status)
    {
        case LIBENCLOUD_HTTP_RC_OK:
            return "OK";
        default:
            return "";
    }
}

}  // namespace libencloud
