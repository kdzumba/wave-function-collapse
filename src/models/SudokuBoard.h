//
// Created by Kdzumba on 2023/06/10.
//

#ifndef WAVE_FUNCTION_COLLAPSE_SUDOKUBOARD_H
#define WAVE_FUNCTION_COLLAPSE_SUDOKUBOARD_H

#include<vector>
#include <tuple>
#include <cmath>
#include <memory>
#include <string>
#include "SudokuBlock.h"

class SudokuBoard
{
public:
    SudokuBoard();
    explicit SudokuBoard(const std::string& filename);
    void init_solve();
    void reset();
    const std::vector<std::vector<std::unique_ptr<SudokuBlock>>>& solve();
    void print_available_options();
    void print();

private:
    void init_board();
    void read_from_file(const std::string& filename);
    void collapse(SudokuBlock* block);
    SudokuBlock* backtrack();
    bool is_fully_solved() const;
    static std::tuple<int, int> random_coordinate();
    static int generate_random_int(int start, int end);
    bool exchange_previous(SudokuBlock *previous);
    SudokuBlock* least_entropy_block();
    void propagate_collapse_info(int row, int col, const std::unique_ptr<BlockState>& state);
    void propagate_decollapse_info(int row, int col, const std::unique_ptr<BlockState>& state);
    std::vector<std::unique_ptr<BlockState>> get_row_exclusions(int row_number);
    std::vector<std::unique_ptr<BlockState>> get_col_exclusions(int col_number);
    std::vector<std::unique_ptr<BlockState>> get_sqr_exclusions(int row_number, int col_number);
    bool is_safe(int row, int col, const std::unique_ptr<BlockState>& state);

private:
    std::vector<std::vector<std::unique_ptr<SudokuBlock>>> m_board;
    std::vector<std::vector<std::unique_ptr<SudokuBlock>>> m_initial_state;
    SudokuBlock* m_current_collapsed;
    std::string m_puzzle_file;
    static int constexpr BOARD_SIZE = 9;
    static int constexpr MIN_FULL_BLOCK_SIZE = 3;
    static int s_stack_counter;
    static int s_retries_count;
    static int constexpr MAX_GENERATE_RETRIES = 200000;
    static int constexpr MAX_RETRIES = 5;
};

#endif //WAVE_FUNCTION_COLLAPSE_SUDOKUBOARD_H
