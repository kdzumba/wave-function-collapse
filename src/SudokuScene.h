//
// Created by Knowledge on 2023/06/08.
//

#ifndef WAVE_FUNCTION_COLLAPSE_SUDOKUSCENE_H
#define WAVE_FUNCTION_COLLAPSE_SUDOKUSCENE_H


#include <QGraphicsScene>
#include "SudokuTile.h"
#include <vector>
#include <memory>

class SudokuScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit SudokuScene();
    [[nodiscard]] std::vector<std::vector<SudokuTile*>> GetSudokuTiles() const;
private:
    static int generateRandom();
private:
    std::vector<std::vector<SudokuTile*>> m_tiles;
    static int constexpr BOARD_SIZE = 9;
};


#endif //WAVE_FUNCTION_COLLAPSE_SUDOKUSCENE_H
