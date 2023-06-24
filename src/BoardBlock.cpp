//
// Created by User on 2023/06/10.
//

#include <algorithm>
#include "BoardBlock.h"

BoardBlock::BoardBlock():m_previous(nullptr), m_current_block(false), m_collapsed_value(0), m_coordinate(std::make_tuple(0,0))
{
    m_available_options = {1, 2, 3, 4, 5, 6, 7, 8, 9};
}

std::vector<int>& BoardBlock::get_available_options()
{
    return m_available_options;
}

int BoardBlock::get_collapsed_value() const
{
    return m_collapsed_value;
}


/**
 * When a block collapses into a single value, we need to remove that value from the block's available
 * options
 * @param value The value that the block was collapsed to
 */
void BoardBlock::set_collapsed_value(int value)
{
    m_collapsed_value = value;
    remove_option(value);
}


/**
 *
 * @return The number of available options it has for non-current blocks and INT_MAX for the current block
 */
unsigned int BoardBlock::get_entropy() const
{
    if(this -> m_current_block)
        return INT_MAX;
    return m_available_options.size();
}


void BoardBlock::remove_option(int value)
{
    //erase-remove idiom: https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
    m_available_options.erase(std::remove(m_available_options.begin(), m_available_options.end(), value),
                              m_available_options.end());
}

bool BoardBlock::operator<(const BoardBlock &other) const
{
    return this -> get_entropy() < other.get_entropy();
}

bool BoardBlock::operator==(const BoardBlock &other) const
{
    return this -> m_collapsed_value == other.m_collapsed_value && m_available_options == other.m_available_options;
}

void BoardBlock::set_coordinate(int x, int y)
{
    m_coordinate = std::make_tuple(x, y);
}

std::tuple<int, int> BoardBlock::get_coordinate()
{
    return m_coordinate;
}

void BoardBlock::set_current_block(bool is_current)
{
    m_current_block = is_current;
}

void BoardBlock::set_previous(BoardBlock *previous)
{
    m_previous = previous;
}

BoardBlock *BoardBlock::get_previous() const
{
    return m_previous;
}

void BoardBlock::add_available_option(int option)
{
    //We don't need to add 0 as an available option (can happen during a de-solve)
    if(option == 0)
        return;

    if(std::find(m_available_options.begin(), m_available_options.end(), option) == m_available_options.end())
        m_available_options.emplace_back(option);
}

void BoardBlock::set_available_options(const std::vector<int>& options)
{
    m_available_options = options;
}
