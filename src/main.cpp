#include <QApplication>
#include <QGraphicsView>
#include "ui/SudokuTile.h"
#include "ui/SudokuScene.h"
#include "models/Board.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto* scene = new SudokuScene();
    auto tiles = scene -> GetSudokuTiles();
    for(const auto& row : tiles){
        for(auto col : row){
            scene ->addItem((SudokuTile*) col);
        }
    }

    auto* view = new QGraphicsView(scene);
    view -> setAlignment(Qt::AlignTop | Qt::AlignLeft);
    view -> show();

    return QApplication::exec();
}
