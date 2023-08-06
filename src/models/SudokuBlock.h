//
// Created by Kdzumba on 2023/06/10.
//

#ifndef WAVE_FUNCTION_COLLAPSE_SUDOKUBLOCK_H
#define WAVE_FUNCTION_COLLAPSE_SUDOKUBLOCK_H


#include <vector>
#include <memory>
#include "Tile.h"

class SudokuBlock
{
public:
    SudokuBlock();
    std::vector<std::unique_ptr<BlockState>>& get_available_states();
    const std::unique_ptr<BlockState>& get_collapsed_state() const;
    void set_collapsed_state(std::unique_ptr<BlockState> option);
    void set_coordinate(int x, int y);
    std::tuple<int,int> get_coordinate();
    void remove_state(const std::unique_ptr<BlockState>& option);
    void make_current_block(bool is_current);
    bool operator< (const SudokuBlock& other) const;
    bool operator== (const SudokuBlock& other) const;
    unsigned int get_entropy() const;
    SudokuBlock* get_previous_block() const;
    void set_previous_block(SudokuBlock* previous);
    void add_available_state(const std::unique_ptr<BlockState>& option);
    void set_available_states(const std::vector<std::unique_ptr<BlockState>>& options);
private:
    SudokuBlock* m_previous;
    bool m_is_current_block;
    std::unique_ptr<BlockState> m_collapsed_state;
    std::tuple<int, int> m_coordinate;
    std::vector<std::unique_ptr<BlockState>> m_available_states;
};


#endif //WAVE_FUNCTION_COLLAPSE_SUDOKUBLOCK_H
