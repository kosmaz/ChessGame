#include "GuiDisplay.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{    
    QApplication a(argc, argv);
    a.addLibraryPath(QCoreApplication::applicationDirPath()+"/plugins");
    GuiDisplay* game=new GuiDisplay;
    game->show();
    return a.exec();
}
