#include <QApplication>
#include "ui/SudokuTile.h"
#include "models/Board.h"
#include "WFCApplication.h"
#include "ui/WorkSpace.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto* application = new WFCApplication();
    auto* workspace = new WorkSpace();

    application -> get_application_window() -> setCentralWidget(workspace -> get_graphics_view());
    application -> get_application_window() -> showMaximized();
    return QApplication::exec();
}
