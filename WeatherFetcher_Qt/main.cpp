#include "FetcherWindow.hpp"
#include <QMessageBox>
#include <QApplication>
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);    

    QSharedMemory shared("MyUniqueAppKey");

    if (!shared.create(512, QSharedMemory::ReadWrite))
    {
        qWarning() << "Can't start more than one instance of the application.";
        return 0;
    }

    const char* server = argc == 2 ? argv[1] : "tcp://localhost:5555";

    FetcherWindow window(server);

    window.show();

    return application.exec();
}