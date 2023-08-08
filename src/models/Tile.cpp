//
// Created by User on 2023/08/04.
//

#include "Tile.h"

BlockState::BlockState(int value): m_value(value) {}

int BlockState::get_value() const { return m_value; }

bool BlockState::operator==(const BlockState &other) const
{
    return this -> m_value == other.m_value;
}

bool BlockState::operator!=(const BlockState &other) const {
    return !(*this == other);
}
