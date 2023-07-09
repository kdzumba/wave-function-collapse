//
// Created by Kdzumba on 2023/06/10.
//

#ifndef WAVE_FUNCTION_COLLAPSE_BOARD_H
#define WAVE_FUNCTION_COLLAPSE_BOARD_H

#include<vector>
#include <tuple>
#include <cmath>
#include <memory>
#include "BoardBlock.h"

using SudokuBoard = std::vector<std::vector<std::unique_ptr<BoardBlock>>>;

class Board
{
public:
    Board();
    explicit Board(const std::string& filename);
    void init_solve();
    void reset();
    const SudokuBoard& solve();
    void print_available_options();
    void print();

private:
    void init_board();
    void read_from_file(const std::string& filename);
    void collapse(BoardBlock* block);
    BoardBlock* backtrack();
    bool is_fully_solved() const;
    static std::tuple<int, int> random_coordinate();
    static int generate_random_int(int start, int end);
    bool exchange_previous(BoardBlock *previous);
    BoardBlock* least_entropy_block();
    void propagate_collapse_info(int row, int col, int value);
    void propagate_decollapse_info(int row, int col, int value);
    std::vector<int> get_row_exclusions(int row_number);
    std::vector<int> get_col_exclusions(int col_number);
    std::vector<int> get_sqr_exclusions(int row_number, int col_number);
    bool is_safe(int row, int col, int value);

private:
    std::vector<std::vector<std::unique_ptr<BoardBlock>>> m_board;
    BoardBlock* m_current_collapsed;
    static int constexpr BOARD_SIZE = 9;
    static int constexpr MIN_FULL_BLOCK_SIZE = 3;
    static int s_stack_counter;
    static int s_retries_count;
    static int constexpr MAX_GENERATE_RETRIES = 10000;
    static int constexpr MAX_RETRIES = 5;
};

#endif //WAVE_FUNCTION_COLLAPSE_BOARD_H
