#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QStringList>
#include <QVariantMap>
#include <encloud/Client>
#include "client.h"

void usage (const QStringList &args);
void addStrOpt (QVariantMap &opts, const QString &opt, const QString &val);

/**
 * Implements a very small subset of Curl functionality in Qt to communicate with Encloud Service.
 */
int main (int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qurl::Client client;
    QStringList args = app.arguments();
    QVariantMap opts;

    if (args.count() < 2)
    {
        usage(args);
        return EXIT_FAILURE;
    }

    QStringListIterator argIt(args);
    QString url;

    qDebug() << "args:" << args;

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
            if (url != "")
            {
                qDebug() << "url was already defined: " << url << "!\n";
                usage(args);
                return EXIT_FAILURE;
            }
            url = arg;
        }
        else if (arg == "-d")
        {
            if (!argIt.hasNext())
            {
                qDebug() << "-d requires a parameter!\n";
                usage(args);
                return EXIT_FAILURE;
            }
            addStrOpt(opts, "postData", argIt.next());
        }
        else if (arg == "-h")
        {
            usage(args);
        }
        else if (arg == "-H")
        {
            if (!argIt.hasNext())
            {
                qDebug() << "-H requires a parameter!\n";
                usage(args);
                return EXIT_FAILURE;
            }
            addStrOpt(opts, "headers", argIt.next());
        }
        else
        {
            qDebug() << "unknown option: " << arg << "!\n";
            usage(args);
            return EXIT_FAILURE;
        }
    }

    if (url == "")
    {
        qDebug() << "url not defined!\n";
        usage(args);
        return EXIT_FAILURE;
    }

    if (!client.run(url, opts))
    {
        qDebug() << "Failed running client!\n";
        return EXIT_FAILURE;
    }

    return app.exec();
}

void usage (const QStringList &args)
{
    qDebug() << "Usage:" << qPrintable(QFileInfo(args[0]).baseName()) << "[options...] <url>";
    qDebug() << "Options:";
    qDebug() << " -d    HTTP POST data";
    qDebug() << " -h    Print out this help";
    qDebug() << " -H    Custom header to pass to server";
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
