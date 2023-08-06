//
// Created by Knowledge on 2023/06/08.
//

#include "SudokuScene.h"
#include "../models/SudokuBoard.h"
#include <random>
#include <vector>

//TODO: dimensions shouldn't be greater than 9, do I even need this to be customizable?
SudokuScene::SudokuScene()
{
    auto board = SudokuBoard("puzzles/puzzle2.txt");
    board.init_solve();
    const auto& result_board = board.solve();

    for(auto n = 0; n < BOARD_SIZE; n++)
    {
        std::vector<SudokuTile*> row;
        for(auto m = 0; m < BOARD_SIZE; m++)
        {
            auto value = result_board.at(n).at(m)->get_collapsed_state() -> get_value();
            row.emplace_back(new SudokuTile(QString::number(value), n * SudokuTile::TILE_SIZE, m * SudokuTile::TILE_SIZE));
        }
        m_tiles.emplace_back(row);
    }

    for(const auto& row : m_tiles){
        for(auto col : row){
            this -> addItem( col);
        }
    }
}

int SudokuScene::generateRandom()
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> uniform_dist(1, 9);
    return uniform_dist(mt);
}

void SudokuScene::animate()
{

}


