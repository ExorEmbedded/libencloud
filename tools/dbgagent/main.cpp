#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QStringList>
#include <QVariantMap>
#include <encloud/Common>
#include <encloud/Crypto>
#include <common/common.h>
#include <common/config.h>

static int run (const QVariantMap &opts);
static void usage (const QStringList &args);
static void addStrOpt (QVariantMap &opts, const QString &opt, const QString &val);


/**
 * Decrypt activation code and provisioning files for debugging purposes
 */
int main (int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    QVariantMap opts;

    QStringListIterator argIt(args);

    // skip on to actual parameters
    argIt.next();

    while (argIt.hasNext())
    {
        QString arg = argIt.next();
        QString next;
        bool isOpt = (arg[0] == '-');

        if (argIt.hasNext())
            next = argIt.peekNext();

        if (!isOpt)
        {
            // no command arguments yet
        }
        else if (arg == "-h")
        {
            usage(args);
            return EXIT_SUCCESS;
        }
        else
        {
            qDebug() << "unknown option: " << arg << "!\n";
            usage(args);
            return EXIT_FAILURE;
        }
    }

    if (run(opts))
    {
        qDebug() << "Failed running client!\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void usage (const QStringList &args)
{
    qDebug() << "Usage:" << qPrintable(QFileInfo(args[0]).baseName()) << "[options...]";
    qDebug() << "Options:";
    qDebug() << " -h    Print out this help";
}

int run (const QVariantMap &opts)
{
    QTextStream out(stdout);
    QByteArray ac;
    QByteArray config;
    libencloud_crypto_t ec;
    
    QFile pf(libencloud::getCommonAppDataDir() + '/' + LIBENCLOUD_REG_PROV_ENC_FILE);

    LIBENCLOUD_ERR_IF (libencloud_crypto_init(&ec));
    LIBENCLOUD_ERR_IF (libencloud_crypto_set_cipher(&ec, libencloud::crypto::Aes256Cfb8Cipher));
    LIBENCLOUD_ERR_IF (libencloud_crypto_set_digest(&ec, libencloud::crypto::Sha256Digest));

    ac = libencloud::getActivationCode(false).toUpper().toAscii();

    out << "<activation_code>" << ac << "</activation_code>" << endl;
    out << endl;

    LIBENCLOUD_ERR_IF (!pf.exists());
    LIBENCLOUD_ERR_IF (!pf.open(QIODevice::ReadOnly));
    config = libencloud::crypto::decrypt(&ec, libencloud::crypto::hash(&ec, ac), pf.readAll());
    LIBENCLOUD_ERR_IF (config.isEmpty());

    out << "<provisioning_file>" << endl;
    out << config << endl;
    out << "</provisioning_file>" << endl;

    return 0;
err:
    return ~0;
}

void addStrOpt (QVariantMap &opts, const QString &opt, const QString &val)
{
    if (opts[opt].isNull())
    {
        opts[opt] = val;
    }
    else
    {
        QStringList list(opts[opt].toStringList());
        list.append(val);
        opts[opt] = list;
    }
}
