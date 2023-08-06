//
// Created by Kdzumba on 2023/06/10.
//

#include "SudokuBoard.h"
#include <random>
#include <algorithm>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>

int SudokuBoard::s_stack_counter = 0;
int SudokuBoard::s_retries_count = 0;

SudokuBoard::SudokuBoard()
{
    init_board();
    m_current_collapsed = nullptr;
}

SudokuBoard::SudokuBoard(const std::string &filename): SudokuBoard{}
{
    m_puzzle_file = filename;
    read_from_file(filename);
}

void SudokuBoard::init_board()
{
    for(auto x = 0; x < BOARD_SIZE; x++)
    {
        auto row = std::vector<std::unique_ptr<SudokuBlock>>();
        for(auto y = 0; y < BOARD_SIZE; y++)
        {
            auto block = std::make_unique<SudokuBlock>();
            block -> set_coordinate(x, y);
            row.emplace_back(std::move(block));
        }
        m_board.emplace_back(std::move(row));
    }
}

void SudokuBoard::init_solve()
{
    auto start_coordinate = random_coordinate();
    int x = std::get<0>(start_coordinate);
    int y = std::get<1>(start_coordinate);

    const auto& initial_block = m_board.at(x).at(y);
    collapse(initial_block.get());
}

/**
 *
 * @param row_number The row number for the currently collapsed block
 * @param col_number The col number for the currently collapsed block
 * @param value  The value that the currently collapsed block was collapsed to
 *
 * Once a block has collapsed into a single value, all the neighbouring blocks (row, col, 3x3 grid)
 * need to have their available options updated to reflect the change (according to the constraints of the game)
 */
void SudokuBoard::propagate_collapse_info(int row_number, int col_number, const std::unique_ptr<BlockState>& state)
{
    auto row_exclusions = get_row_exclusions(row_number);

    // Remove exclusions from available options of blocks at row_number
    for(const auto& block : m_board.at(row_number))
    {
        block->remove_state(state);
        auto new_states = std::vector<std::unique_ptr<BlockState>>();

        //We need options that haven't been excluded by the current solve, hence set_difference
        std::set_difference(std::move_iterator(block -> get_available_states().begin()), std::move_iterator(block->get_available_states().end()),
                            row_exclusions.begin(), row_exclusions.end(), std::inserter(new_states, new_states.begin()));

        block -> set_available_states(new_states);
    }

    //remove col_exclusions from available_states of column col_number
    auto col_exclusions = get_col_exclusions(col_number);
    for(const auto& _row : m_board)
    {
        const auto& block = _row.at(col_number);
        block -> remove_state(state);
        auto new_states = std::vector<std::unique_ptr<BlockState>>();
        std::set_difference(std::move_iterator(block->get_available_states().begin()), std::move_iterator(block->get_available_states().end()),
                            col_exclusions.begin(), col_exclusions.end(), std::inserter(new_states, new_states.begin()));

        block -> set_available_states(new_states);
    }

    //remove start_value from available_options of 3x3 surrounding block
    int start_row_index = row_number - row_number % MIN_FULL_BLOCK_SIZE;
    int start_col_index = col_number - col_number % MIN_FULL_BLOCK_SIZE;
    auto sqr_exclusions = get_sqr_exclusions(row_number, col_number);

    for(auto m = 0; m < MIN_FULL_BLOCK_SIZE; m++)
    {
        for (auto n = 0; n < MIN_FULL_BLOCK_SIZE; n++)
        {
            const auto& block = m_board.at(m + start_row_index).at(n + start_col_index);
            block -> remove_state(state);
            auto new_states = std::vector<std::unique_ptr<BlockState>>();

            std::set_difference(std::move_iterator(block->get_available_states().begin()), std::move_iterator(block->get_available_states().end()),
                                sqr_exclusions.begin(), sqr_exclusions.end(), std::inserter(new_states, new_states.begin()));
            block->set_available_states(new_states);
        }
    }
}

void SudokuBoard::collapse(SudokuBlock* block)
{
    auto rand_index = generate_random_int(0, (int) block->get_available_states().size() - 1);
    auto next_state = std::make_unique<BlockState>(*(block->get_available_states().at(rand_index)));
    block->set_collapsed_state(std::move(next_state));
    block->make_current_block(true);

    if(m_current_collapsed != nullptr)
        m_current_collapsed->make_current_block(false);

    if(m_current_collapsed != nullptr && block != m_current_collapsed)
        block->set_previous_block(m_current_collapsed);

    m_current_collapsed = block;
    auto row_number = std::get<0>(block -> get_coordinate());
    auto col_number = std::get<1>(block -> get_coordinate());

    propagate_collapse_info(row_number, col_number, block -> get_collapsed_state());
}

/**
 * This method finds the first previously collapsed block that still has other available options that can
 * replace it's current collapsed value
 * @return The next block to be collapsed.
 */
SudokuBlock* SudokuBoard::backtrack()
{
    std::cout << "Backtracking" << std::endl;
    auto current = m_current_collapsed;

    //We need to go back through all previously set blocks until we find one where we could have
    //chosen another alternative
    while(current != nullptr && current -> get_available_states().empty())
    {
        current = current -> get_previous_block();
    }

    if(current == nullptr)
    {
        //If we got here, it means we went all the way back to the first set block, which means the starting block
        //Was not the right one to start with. We need to reset the board and re-solve from scracth.
        //The option of finding another block that is unset doesn't solve the problem of there being a block in this
        //current path that has an entropy of 0
        std::cout << "nullptr scenario occurred" << std::endl;
        return nullptr;
    }

    auto current_collapsed_state = std::make_unique<BlockState>(*( current->get_collapsed_state()));
    current->set_collapsed_state(std::make_unique<BlockState>(0));
    propagate_decollapse_info(std::get<0>(current -> get_coordinate()), std::get<1>(current -> get_coordinate()), current_collapsed_state);

    return current;
}

/**
 * Continuously solves a sudoku puzzle using wave function collapse and backtracking
 * @return A 2-dimensional vector of unique_ptr to SudokuBlock, which represents a fully solved sudoku
 */
const std::vector<std::vector<std::unique_ptr<SudokuBlock>>>& SudokuBoard::solve()
{
    s_stack_counter ++;
    auto next_block = least_entropy_block();

    if(next_block -> get_entropy() == 0)
        next_block = backtrack();

    if(next_block == nullptr)
    {
        if(s_retries_count < MAX_RETRIES)
        {
            std::cout << "Could not solve board, retrying" << std::endl;
            reset();
            init_solve();
        }
        else
        {
            // TODO: Need to handle this scenario by letting the client know that the board can't be solved
            std::cout << "Failed to solve puzzle after " << MAX_RETRIES << " attempts" << std::endl;
            return m_board;
        }
    }
    else
    {
        collapse(next_block);
    }

    print();

    if(!is_fully_solved() && s_stack_counter < MAX_GENERATE_RETRIES)
        solve();

    return m_board;
}

/**
 * Generates a tuple that consists of a row number and column number for a sudoku board [0, 8]
 * @return A tuple representing the coordinate of a block within a sudoku board [0,8]
 */
std::tuple<int, int> SudokuBoard::random_coordinate() {
    int x = generate_random_int(0, 8);
    int y = generate_random_int(0, 8);
    return std::make_tuple(x, y);
}

/**
 * Generates a random integer in the range [start, end]
 * @param start The minimum integer that can be generated
 * @param end The maximum integer that can be generated
 * @return A random integer in the range [start, end]
 */
int SudokuBoard::generate_random_int(int start, int end) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(start, end);
    return distribution(generator);
}

/**
 * Finds the block with the least entropy within a sudoku board
 * @return A pointer to a SudokuBlock with the least entropy, given by SudokuBlock::get_entropy()
 */
SudokuBlock* SudokuBoard::least_entropy_block()
{
    std::map<unsigned int, std::vector<SudokuBlock*>> entropy_to_block_map;
    for(const auto& row : m_board)
    {
        for(const auto & block : row)
        {
            //Only consider blocks that haven't been collapsed yet
            if(block->get_collapsed_state() != nullptr and block->get_collapsed_state() -> get_value() == 0)
            {
                unsigned int key = block -> get_entropy();
                if(entropy_to_block_map.find(key) != entropy_to_block_map.end() && key != 0)
                {
                    entropy_to_block_map.at(key).emplace_back(block.get());
                }
                else
                {
                    auto value = std::vector<SudokuBlock*>{block.get()};
                    entropy_to_block_map.insert({key, value});
                }
            }
        }
    }

    auto min_entropy = entropy_to_block_map.begin() -> second;
    auto rand_index = generate_random_int(0, (int)(min_entropy.size() - 1));
    return min_entropy.at(rand_index);
}

/**
 * @return true if the sudoku board is completely solved, false otherwise
 */
bool SudokuBoard::is_fully_solved() const
{
    bool is_full = true;
    for(const auto& row : m_board)
    {
        for(const auto & col : row)
        {
            if(col->get_collapsed_state() == nullptr || col -> get_collapsed_state() -> get_value() == 0)
                is_full = false;
        }
    }
    return is_full;
}

/**
 * Debug function to see the state of the sudoku board at any point in time
 */
void SudokuBoard::print()
{
    std::cout << "Recursive call count: " << s_stack_counter << std::endl;
    std::cout << std::setw(4) << 0;

    for(auto col : {1, 2, 3, 4, 5, 6, 7, 8})
        std::cout << std::setw(2) << col;
    std::cout << std::endl << "_______________________" << std::endl;

    int row_count = 0;
    for(const auto& row : m_board)
    {
        std::cout << row_count << "| ";
        for(const auto& block : row)
        {
            std::cout << (block->get_collapsed_state()) -> get_value() << " ";
        }
        row_count++;
        std::cout << std::endl;
    }
    printf("\n");
}

/**
 * When a collapse/de-solve happens, we need to re-compute the values that aren't available to blocks in the same row
 * as the collapsed/de-collapsed block
 * @param row_number The row number for which we want to compute the values that aren't available to blocks in that row
 * @return A vector of values that blocks in row_number can't take
 */

std::vector<std::unique_ptr<BlockState>> SudokuBoard::get_row_exclusions(int row_number)
{
    auto row_exclusions = std::vector<std::unique_ptr<BlockState>>();
    for(const auto& block : m_board.at(row_number))
    {
        if(block->get_collapsed_state() != nullptr && block->get_collapsed_state() -> get_value() != 0)
            row_exclusions.emplace_back(std::make_unique<BlockState>(*(block->get_collapsed_state())));
    }

    std::sort(row_exclusions.begin(), row_exclusions.end());
    return row_exclusions;
}

/**
 * When a collapse/de-solve happens, we need to re-compute the values that aren't available to blocks in the same row
 * as the collapsed/de-collapsed block
 * @param col_number The col number for which we want to compute the values that aren't available to blocks in that col
 * @return A vector of values that blocks in col_number can't take
 */
std::vector<std::unique_ptr<BlockState>> SudokuBoard::get_col_exclusions(int col_number)
{
    auto col_exclusions = std::vector<std::unique_ptr<BlockState>>();

    for(auto& _row : m_board)
    {
        const auto& block =  _row.at(col_number);
        if(block->get_collapsed_state() != nullptr && block->get_collapsed_state() -> get_value() != 0)
            col_exclusions.emplace_back(std::make_unique<BlockState>(*(_row.at(col_number)->get_collapsed_state())));
    }
    std::sort(col_exclusions.begin(), col_exclusions.end());
    return col_exclusions;
}

/**
 *
 * @param row_number The row number for the collapsed item
 * @param col_number The col number for the collapsed item
 * @return A vector of values that blocks in the same 3x3 block as (row,col) can't take
 */
std::vector<std::unique_ptr<BlockState>> SudokuBoard::get_sqr_exclusions(int row_number, int col_number)
{
    auto sqr_exclusions = std::vector<std::unique_ptr<BlockState>>();

    int start_row_index = row_number - row_number % MIN_FULL_BLOCK_SIZE;
    int start_col_index = col_number - col_number % MIN_FULL_BLOCK_SIZE;

    for(auto m = 0; m < MIN_FULL_BLOCK_SIZE; m++)
    {
        for (auto n = 0; n < MIN_FULL_BLOCK_SIZE; n++)
        {
            const auto& block = m_board.at(m + start_row_index).at(n + start_col_index);
            if(block->get_collapsed_state() != nullptr && block->get_collapsed_state() -> get_value() != 0)
                sqr_exclusions.emplace_back(std::make_unique<BlockState>(*(block->get_collapsed_state())));
        }
    }
    std::sort(sqr_exclusions.begin(), sqr_exclusions.end());
    return sqr_exclusions;
}


/**
 *
 * @param row The row number for the de-collapsed block
 * @param col The col number for the de-collapsed block
 * @param value The value the block was de-collapsed to
 * During backtracking, we need to reset some blocks that were previously collapsed before the current block. Doing so
 * entails putting the block's value back to available_options for that block
 */
void SudokuBoard::propagate_decollapse_info(int row, int col, const std::unique_ptr<BlockState>& value)
{
    const auto& being_decollapsed = m_board.at(row).at(col).get();
    auto has_exchanged = exchange_previous(being_decollapsed);

    //Every block in row should get value added to their available options
    for(const auto& block : m_board.at(row))
    {
        auto current_row = std::get<0>(block -> get_coordinate());
        auto current_col = std::get<1>(block -> get_coordinate());

        if (!(current_row == row && current_col == col) && is_safe(current_row, current_col, value))
        {
            block->add_available_state(value);
        }
    }

    //Every block in col should get value added back to their available options
    for(const auto& _row : m_board)
    {
        const auto& block = _row.at(col);
        auto current_row = std::get<0>(block -> get_coordinate());
        auto current_col = std::get<1>(block -> get_coordinate());

        if(!(current_row == row && current_col == col) && is_safe(current_row, current_col, value))
            block->add_available_state(value);
    }

    int start_row_index = row - row % MIN_FULL_BLOCK_SIZE;
    int start_col_index = col - col % MIN_FULL_BLOCK_SIZE;

    for(auto m = 0; m < MIN_FULL_BLOCK_SIZE; m++)
    {
        for(auto n = 0; n < MIN_FULL_BLOCK_SIZE; n++)
        {
            const auto& block = m_board.at(m + start_row_index).at(n + start_col_index);
            auto current_row = std::get<0>(block -> get_coordinate());
            auto current_col = std::get<1>(block -> get_coordinate());

            if(!(current_row == row && current_col == col) && is_safe(current_row, current_col, value))
                block->add_available_state(value);
        }
    }

    if(has_exchanged)
        being_decollapsed->set_previous_block(nullptr);
}


/**
 * Only used for debugging to see available options for blocks at a certain point in the code
 */
void SudokuBoard::print_available_options()
{
    for(const auto& row : m_board)
    {
        for(const auto& block : row)
        {
            auto current_x = std::get<0>(block -> get_coordinate());
            auto current_y = std::get<1>(block -> get_coordinate());
            std::cout << "(" << current_x << " , " << current_y << ")" << " --> " << "{";
            for(const auto& item : block->get_available_states())
                std::cout << item -> get_value() << " ";
            std::cout << "}" << std::endl;
        }
        std::cout << std::endl;
    }
    printf("\n");
}

/**
 * Before adding a value back to a block's available options, we need to check if it's still a safe option for the block
 * as this might have changed due to some change that has already happened on the board
 * @param row The row number at which the block is found
 * @param col The col number at which the block is found
 * @param value The value we'd like to place back into available options for the block
 * @return True if placing the option doesn't violate constraints of the game, false otherwise
 */
bool SudokuBoard::is_safe(int row, int col, const std::unique_ptr<BlockState>& value)
{
    auto row_exclusions = get_row_exclusions(row);
    if(std::find(row_exclusions.begin(), row_exclusions.end(), value) != row_exclusions.end())
        return false;

    auto col_exclusions = get_col_exclusions(col);
    if(std::find(col_exclusions.begin(), col_exclusions.end(), value) != col_exclusions.end())
        return false;

    auto sqr_exclusions = get_sqr_exclusions(row, col);
    if(std::find(sqr_exclusions.begin(), sqr_exclusions.end(), value) != sqr_exclusions.end())
        return false;
    return true;
}

/**
 * During de-collapse, we need to update the previous block for all the blocks whose previous was the block that's
 * being de-collapsed
 * @param previous The block being de-collapsed
 * @return True if an exchange happened, false otherwise.
 */
bool SudokuBoard::exchange_previous(SudokuBlock *previous)
{
    for(const auto& row : m_board)
    {
        for(const auto& block : row)
        {
            if(block->get_previous_block() == previous) {
                std::cout << "Swapping occurred" << std::endl;
                block->set_previous_block(previous->get_previous_block());
                return true;
            }
        }
    }
    return false;
}

void SudokuBoard::read_from_file(const std::string &filename)
{
    std::ifstream input(filename);
    if(!input.good())
    {
        std::cerr << "Error: could not open file: " << filename << std::endl;
        exit(1);
    }

    std::string input_row;
    int row_index = 0;
    int col_index = 0;

    while(std::getline(input, input_row))
    {
        std::stringstream ss(input_row);
        auto row = std::vector<std::unique_ptr<SudokuBlock>>();

        while(ss.good() && col_index < BOARD_SIZE)
        {
            int value;
            ss >> value;
            auto state = std::make_unique<BlockState>(value);

            const auto& block = m_board.at(row_index).at(col_index);
            block->set_collapsed_state(std::move(state));
            propagate_collapse_info(row_index, col_index, block->get_collapsed_state());
            col_index++;
        }
        row_index++;
        col_index = 0;
    }
    input.close();
}

void SudokuBoard::reset()
{
    m_board.clear();
    init_board();
    m_current_collapsed = nullptr;
    s_stack_counter = 0;
    s_retries_count++;

    //TODO: Don't like having to read from file again, will need to find a way to save the initial state
    read_from_file(m_puzzle_file);
}
