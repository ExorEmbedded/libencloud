#include <common/common.h>

namespace libencloud {
namespace info {

QString org () 
{ 
    return LIBENCLOUD_ORG;
}

QString product () 
{ 
    return LIBENCLOUD_PRODUCT; 
}

QString app () 
{ 
    return LIBENCLOUD_PKGNAME; 
}

QString version () 
{ 
    return LIBENCLOUD_VERSION; 
}

QString revision () 
{ 
    return LIBENCLOUD_REVISION; 
}

QString versionTag () 
{ 
    return LIBENCLOUD_VERSION_TAG; 
}

QString versionInfo ()
{
    QString info;

    info += org() + " ";

    if (product() != "")
        info += product() + " ";

    info += app() + " v" + version();

    // revision is printed only to logs (not very user-friendly)
    //if (revision() != "")
    //    info += " rev " + revision();

    if (versionTag() != "")
        info += " (" + versionTag() + ")";

    return info;
}

}  // namespace info
}  // namespace libencloud
