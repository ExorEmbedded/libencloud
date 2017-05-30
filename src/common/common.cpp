#include <QObject>
#include <QCoreApplication>
#ifdef Q_OS_WIN32
#  include <shlobj.h>
#endif
#include <encloud/Common>
#include <encloud/Utils>
#include <encloud/simplecrypt/simplecrypt.h>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

static const QString sep = "/";

LIBENCLOUD_DLLSPEC QString paramsFind (const Params &params, const QString &key)
{
    foreach (Param param, params)
        if (param.first == key)
            return param.second;

    return "";
}

/* \brief Where to look for application binaries */
LIBENCLOUD_DLLSPEC QString getBinDir ()
{
#ifndef LIBENCLOUD_SPLITDEPS
    return (qApp ? qApp->applicationDirPath() : QDir::currentPath()) + sep;
#else
    return QString(LIBENCLOUD_BIN_PREFIX);
#endif
}

LIBENCLOUD_DLLSPEC QString getCommonAppDataDir (QString package)
{
    QString s;

#ifndef LIBENCLOUD_SPLITDEPS

#if defined Q_OS_WIN32
    char szPath[MAX_PATH];
    // compiled for unicode => use ANSI version
    LIBENCLOUD_RETURN_IF (SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath),
            QString());
    s += QString(szPath);
#elif defined Q_OS_MAC || defined Q_OS_UNIX
    s += LIBENCLOUD_DATA_PREFIX;
#endif

    s += sep + QString(LIBENCLOUD_PRODUCTDIR) + sep;
    if (package == "")
        s += QString(LIBENCLOUD_PKGNAME);
    else
        s += package;
    s += sep;

    return s;

#else
    LIBENCLOUD_UNUSED(package);
    LIBENCLOUD_UNUSED(s);

    return QString(LIBENCLOUD_DATA_PREFIX);
#endif
}

LIBENCLOUD_DLLSPEC QString getCommonLogDir (QString package)
{
#ifndef LIBENCLOUD_SPLITDEPS
    return getCommonAppDataDir(package);
#else
    LIBENCLOUD_UNUSED(package);
    return QString(LIBENCLOUD_LOG_PREFIX);
#endif
}

// Agent mode
LIBENCLOUD_DLLSPEC QString getActivationCode (bool encrypted)
{
    SimpleCrypt crypto(QICC_SETTING_KEY);
    QString s, e;

    QFile activationCodeFile(getCommonAppDataDir() + "activation-code");
    QFile activationCodeFileEnc(getCommonAppDataDir() + "activation-code.enc");

    // temporary unencrypted file (can be used for configuration)
    if (activationCodeFile.open(QFile::ReadOnly))
    {
        s = activationCodeFile.readLine().trimmed();
        LIBENCLOUD_ERR_IF (s.isEmpty());

        // create encrypted version and remove original
        LIBENCLOUD_ERR_IF ((e = setActivationCode(s, true)).isEmpty());
        activationCodeFile.remove();

        return (encrypted ? e : s);
    }

    // encrypted file
    LIBENCLOUD_ERR_IF (!activationCodeFileEnc.open(QFile::ReadOnly));

    e = activationCodeFileEnc.readLine().trimmed();
    LIBENCLOUD_ERR_IF (e.isEmpty());

    return (encrypted ? e : crypto.decryptToString(e));

err:
    return QString();
}

LIBENCLOUD_DLLSPEC QString setActivationCode (const QString &code, bool encrypt)
{
    SimpleCrypt crypto(QICC_SETTING_KEY);

    LIBENCLOUD_DBG("Setting activation code: " << code << ", encrypt: " << encrypt);

    QString path = getCommonAppDataDir() + "activation-code.enc";

    if (code == QString(utils::file2Data(QFileInfo(path))))
        return code;

    LIBENCLOUD_DBG("Writing to: " << path);

    QByteArray ba;

    if (encrypt)
        ba = crypto.encryptToString(code).toAscii();
    else
        ba = code.toAscii();

    LIBENCLOUD_ERR_IF (utils::bytes2File(ba, path, true));

    return ba;
err:
    return QString();
}

}  // namespace libencloud
