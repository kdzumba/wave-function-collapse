//
// Created by User on 2023/07/09.
//

#include "WorkSpace.h"
#include "SudokuScene.h"

int WorkSpace::s_current_state_index = 0;

WorkSpace::WorkSpace()
{
    m_graphics_view = new QGraphicsView(this);
    m_graphics_view -> setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    init_default_scenes();
    m_graphics_view -> setScene(m_scene_states.at(s_current_state_index).get());
}

void WorkSpace::init_default_scenes()
{
    auto sudoku_scene = std::make_unique<SudokuScene>();
    m_scene_states.emplace_back(std::move(sudoku_scene));
}

QGraphicsView *WorkSpace::get_graphics_view() {
    return m_graphics_view;
}
