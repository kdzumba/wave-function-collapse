#include <QApplication>
#include <QGraphicsView>
#include "MainApplicationWindow.h"
#include "SudokuTile.h"
#include "SudokuScene.h"
#include "SudokuBoard.h"
#include "models/Board.h"

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
auto board = Board("puzzles/puzzle3.txt");
board.init_solve();
board.solve();
board.print();
//board.init_solve();
//board.solve();
//    QGraphicsView view(scene);
//    view.show();

    return QApplication::exec();
}
