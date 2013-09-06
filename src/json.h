#ifndef _ECE_JSON_H_
#define _ECE_JSON_H_

#include <QVariant>
#include <QString>

#ifndef ECE_DLLSPEC
#  if defined(Q_OS_WIN)
#    if defined _ECELIB_
#       define ECE_DLLSPEC __declspec(dllexport)
#    else
#        define ECE_DLLSPEC __declspec(dllimport)
#    endif
#else
#    define ECE_DLLSPEC
#endif
#endif

namespace EceJson {

    // implementation-specific interfaces (defined in json-xxx.cpp)
    ECE_DLLSPEC QVariant parse (const QString &str, bool &ok);
    ECE_DLLSPEC QString serialize (const QVariant &json, bool &ok);

    // generic helpers (defined in json.cpp)
    ECE_DLLSPEC QVariant parseFromFile (const QString &filename, bool &ok);
}

#endif
