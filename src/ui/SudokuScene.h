//
// Created by Knowledge on 2023/06/08.
//

#ifndef WAVE_FUNCTION_COLLAPSE_SUDOKUSCENE_H
#define WAVE_FUNCTION_COLLAPSE_SUDOKUSCENE_H


#include <QGraphicsScene>
#include "SudokuTile.h"
#include "../models/BoardBlock.h"
#include "AbstractScene.h"
#include <vector>
#include <memory>

class SudokuScene : public AbstractScene
{
    Q_OBJECT
public:
    explicit SudokuScene();
private:
    static int generateRandom();
private:
    std::vector<std::vector<SudokuTile*>> m_tiles;
    static int constexpr BOARD_SIZE = 9;
    std::vector<std::vector<std::unique_ptr<BoardBlock>>> m_board;
};


#endif //WAVE_FUNCTION_COLLAPSE_SUDOKUSCENE_H
