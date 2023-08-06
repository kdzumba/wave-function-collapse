//
// Created by User on 2023/06/10.
//

#include <algorithm>
#include <utility>
#include "SudokuBlock.h"

SudokuBlock::SudokuBlock(): m_previous(nullptr), m_is_current_block(false), m_coordinate(std::make_tuple(0, 0))
{
    for(int i = 1; i <= 9; i++)
    {
        auto block = std::make_unique<BlockState>(i);
        m_available_states.emplace_back(std::move(block));
    }
}

std::vector<std::unique_ptr<BlockState>>& SudokuBlock::get_available_states()
{
    return m_available_states;
}


const std::unique_ptr<BlockState>& SudokuBlock::get_collapsed_state() const
{
    return m_collapsed_state;
}


/**
 * When a block collapses into a single value, we need to remove that value from the block's available
 * options
 * @param value The value that the block was collapsed to
 */
void SudokuBlock::set_collapsed_state(std::unique_ptr<BlockState> state)
{
    remove_state(state);
    m_collapsed_state = std::move(state);
}


/**
 *
 * @return The number of available options it has for non-current blocks and INT_MAX for the current block
 */
unsigned int SudokuBlock::get_entropy() const
{
    if(this -> m_is_current_block)
        return INT_MAX;
    return m_available_states.size();
}

void SudokuBlock::remove_state(const std::unique_ptr<BlockState>& state)
{
    m_available_states.erase(std::remove_if(m_available_states.begin(), m_available_states.end(),
                                            [&](const std::unique_ptr<BlockState>& s) -> bool{
        return state != nullptr  && s != nullptr && state -> get_value() == s -> get_value();
    }), m_available_states.end());
}

bool SudokuBlock::operator<(const SudokuBlock &other) const
{
    return this -> get_entropy() < other.get_entropy();
}

bool SudokuBlock::operator==(const SudokuBlock &other) const
{
    return this -> m_collapsed_state == other.m_collapsed_state && m_available_states == other.m_available_states;
}

void SudokuBlock::set_coordinate(int x, int y)
{
    m_coordinate = std::make_tuple(x, y);
}

std::tuple<int, int> SudokuBlock::get_coordinate()
{
    return m_coordinate;
}

void SudokuBlock::make_current_block(bool is_current)
{
    m_is_current_block = is_current;
}

void SudokuBlock::set_previous_block(SudokuBlock *previous)
{
    m_previous = previous;
}

SudokuBlock *SudokuBlock::get_previous_block() const
{
    return m_previous;
}

void SudokuBlock::add_available_state(const std::unique_ptr<BlockState>& option)
{
    //We don't need to add 0 as an available option (can happen during a de-solve)
    if(option -> get_value() == 0)
        return;

    auto option_found = [&](const std::unique_ptr<BlockState>& opt) -> bool {
        return opt -> get_value() == option -> get_value();
    };

    if(std::find_if(m_available_states.begin(), m_available_states.end(), option_found) == m_available_states.end())
        m_available_states.emplace_back(std::make_unique<BlockState>(*option));
}

/**
 * We want to update the available options for this SudokuBlock so that it now contains the same elements as those in
 * the options vector
 * @param options Vector of unique_ptr to BlockState that this SudokuBlock's available options will be set to
 */
void SudokuBlock::set_available_states(const std::vector<std::unique_ptr<BlockState>>& options)
{
    m_available_states.clear();
    for(const auto& option : options)
    {
        m_available_states.emplace_back(std::make_unique<BlockState>(*option));
    }
}
