#include "libraryhandler.h"

LibraryHandler::LibraryHandler(QObject *parent) : QObject(parent)
{
    librariesDir = QDir("mslibs").absolutePath();
}

QLibrary *LibraryHandler::LoadPlugin(QString name, QString library)
{
    if (libraries.contains(name))
    {
        return libraries[name];
    }

    auto lib = new QLibrary(library);
    if (lib->load())
    {
        typedef bool *(*func)();
        auto init = reinterpret_cast<func>(lib->resolve("init"));
        if (!init || !init()) {
            return nullptr;
        }

        libraries[name] = lib;
        return lib;
    }
    else
    {
        qInfo() << "Could not load plugin" << library;
        return nullptr;
    }
}

QString LibraryHandler::DebugDir(QString name)
{
    return QDir("../" + name + "/debug").absolutePath();
}

QLibrary *LibraryHandler::CemuDb()
{
#if defined(QT_DEBUG)
    librariesDir = DebugDir("cemudb");
#endif
    QLibrary *library;
    if ((library = LoadPlugin("cemudb", librariesDir + "/cemudb")))
    {
        return library;
    }
    return nullptr;
}

CemuDatabase *LibraryHandler::newCemuDatabase()
{
    QLibrary *library;
    if ((library = CemuDb()))
    {
        typedef CemuDatabase *(*func)();
        func function = reinterpret_cast<func>(library->resolve("initialize"));
        if (function)
        {
            return function();
        }
        else {
            qCritical() << "Unable to load CemuDatabase from CemuLib";
            return nullptr;
        }
    }
    return nullptr;
}
