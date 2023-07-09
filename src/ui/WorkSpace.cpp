//
// Created by User on 2023/07/09.
//

#include "WorkSpace.h"
#include "SudokuScene.h"

int WorkSpace::s_current_state_index = 0;

WorkSpace::WorkSpace()
{
    this -> setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    init_default_scenes();
    this -> setScene(m_scene_states.at(s_current_state_index).get());
}

void WorkSpace::init_default_scenes()
{
    auto sudoku_scene = std::make_unique<SudokuScene>();
    m_scene_states.emplace_back(std::move(sudoku_scene));
}
