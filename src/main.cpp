#include <QApplication>
#include <QGraphicsView>
#include "MainApplicationWindow.h"
#include "SudokuTile.h"
#include "SudokuScene.h"
#include "SudokuBoard.h"
#include "Board.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    auto* scene = new SudokuScene();
//    auto tiles = scene -> GetSudokuTiles();
//    for(const auto& row : tiles){
//        for(auto col : row){
//            scene ->addItem((SudokuTile*) col);
//        }
//    }

//    auto sudoku = SudokuBoard(17);
auto board = Board();
//    QGraphicsView view(scene);
//    view.show();

    return QApplication::exec();
}
