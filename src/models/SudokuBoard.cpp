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
    m_initial_block = nullptr;
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
    int x = 2; //std::get<0>(start_coordinate);
    int y = 8;//std::get<1>(start_coordinate);

//    const auto& initial_block = m_board.at(x).at(y);
    collapse(get_initial_block());
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
    std::cout << "Before" << std::endl;
    std::cout << "Setting block: x: " << std::get<0>(block -> get_coordinate()) << " , y: " << std::get<1>(block -> get_coordinate()) << std::endl;
    print();

    auto rand_index = generate_random_int(0, (int) block->get_available_states().size() - 1);
    auto next_state = std::make_unique<BlockState>(*(block->get_available_states().at(rand_index)));

    //Some of the blocks are already default-collapsed from reading the puzzle files, and these shouldn't be collapsed to
    //something different. least_entropy_block() shouldn't consider them either, but checking if m_collapsed_state is a nullptr
    //should take care of this in least_entropy_block()
    if(!(block->get_is_permanently_collapsed()))
        block->set_collapsed_state(std::move(next_state));

    //The current_block concept is used to prevent the currently collapsed block from being picked as the least_entropy_block
    block->make_current_block(true);

    //The initial block doesn't have a previous as it's the first block to be processed
    if(block != m_initial_block && block != m_current_collapsed)
        block->set_previous_block(m_current_collapsed);

    if(block -> get_previous_block() == block)
        std::cout << "Pointing to itself" << std::endl;

    auto row_number = std::get<0>(block -> get_coordinate());
    auto col_number = std::get<1>(block -> get_coordinate());

    if(m_current_collapsed == block)
        std::cout << "current_collapsed and block to collapse are the same" << std::endl;

    m_current_collapsed = block;
    propagate_collapse_info(row_number, col_number, block -> get_collapsed_state());
    std::cout << "After: " << std::endl;
    print();
}

/**
 * This method finds the first previously collapsed block that still has other available options that can
 * replace it's current collapsed value
 * @return The next block to be collapsed.
 */
SudokuBlock* SudokuBoard::backtrack()
{
    std::cout << "Backtracking" << std::endl;
    auto to_reprocess = m_current_collapsed;

    //We need to go back through all previously set blocks until we find one where we could have
    //chosen another alternative
    while(to_reprocess != nullptr && to_reprocess -> get_available_states().empty())
    {
        to_reprocess = to_reprocess -> get_previous_block();
    }

    if(to_reprocess == m_current_collapsed)
        std::cout << "m_current_collapsed to be re_processed" << std::endl;

    if(to_reprocess == nullptr)
    {
        //If we got here, it means we went all the way back to the first set block, which means the starting block
        //Was not the right one to start with. We need to reset the board and re-solve from scratch.
        //The option of finding another block that is unset doesn't solve the problem of there being a block in this
        //to_reprocess path that has an entropy of 0
        std::cout << "nullptr scenario occurred" << std::endl;
        return nullptr;
    }

    //Getting here means we found a block in the to_reprocess path(going backwards) that had more options to choose from
    //then the one it collapsed to. We need to first de-collapse the to_reprocess block (set it to de-collapsed state, and
    //propagate this de-collapse to other blocks so that they may also have this de-collapsed state in their available
    //states again)
    auto old_collapsed_state = std::make_unique<BlockState>(*( to_reprocess->get_collapsed_state()));
    to_reprocess->set_collapsed_state(std::make_unique<BlockState>(0));
    propagate_decollapse_info(std::get<0>(to_reprocess -> get_coordinate()), std::get<1>(to_reprocess -> get_coordinate()), old_collapsed_state);

    return to_reprocess;
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

//    print();

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
            if(!(block -> get_is_permanently_collapsed()))
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

    auto least_entropy_block = min_entropy.at(rand_index);
    if(least_entropy_block == m_current_collapsed)
        std::cout << "current_collapsed and least_entropy are the same" << std::endl;

    return least_entropy_block;
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
 * @param state The state the block was de-collapsed to
 * During backtracking, we need to reset some blocks that were previously collapsed before the current block. Doing so
 * entails putting the block's state back to available_options for that block
 */
void SudokuBoard::propagate_decollapse_info(int row, int col, const std::unique_ptr<BlockState>& state)
{
    const auto& being_decollapsed = m_board.at(row).at(col).get();
    update_processing_chain(being_decollapsed);

    //Every block in row should get state added to their available options
    for(const auto& block : m_board.at(row))
    {
        auto current_row = std::get<0>(block -> get_coordinate());
        auto current_col = std::get<1>(block -> get_coordinate());

        if (!(row == current_row && col == current_col) && is_safe(current_row, current_col, state))
        {
            block->add_available_state(state);
        }
    }

    //Every block in col should get state added back to their available options
    for(const auto& _row : m_board)
    {
        const auto& block = _row.at(col);
        auto current_row = std::get<0>(block -> get_coordinate());
        auto current_col = std::get<1>(block -> get_coordinate());

        if(!(row == current_row && col == current_col) && is_safe(current_row, current_col, state))
            block->add_available_state(state);
    }

    //Every block in the 3x3 square grid should get state added back to their available options
    int start_row_index = row - row % MIN_FULL_BLOCK_SIZE;
    int start_col_index = col - col % MIN_FULL_BLOCK_SIZE;

    for(auto m = 0; m < MIN_FULL_BLOCK_SIZE; m++)
    {
        for(auto n = 0; n < MIN_FULL_BLOCK_SIZE; n++)
        {
            const auto& block = m_board.at(m + start_row_index).at(n + start_col_index);
            auto current_row = std::get<0>(block -> get_coordinate());
            auto current_col = std::get<1>(block -> get_coordinate());

            if(!(row == current_row && col == current_col) && is_safe(current_row, current_col, state))
                block->add_available_state(state);
        }
    }
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
 * Before adding a state back to a block's available options, we need to check if it's still a safe option for the block
 * as this might have changed due to some change that has already happened on the board
 * @param row The row number at which the block is found
 * @param col The col number at which the block is found
 * @param state The state we'd like to place back into available options for the block
 * @return True if placing the option doesn't violate constraints of the game, false otherwise
 */
bool SudokuBoard::is_safe(int row, int col, const std::unique_ptr<BlockState>& state)
{
    auto row_exclusions = get_row_exclusions(row);
    bool is_row_safe = true, is_col_safe = true, is_block_safe = true;

    auto is_state_excluded = [&](const std::unique_ptr<BlockState>& s) -> bool {
        return state -> get_value() == s -> get_value();
    };

    auto row_safe_iter = std::find_if(row_exclusions.begin(), row_exclusions.end(),is_state_excluded);
    if(row_safe_iter != row_exclusions.end())
        is_row_safe = false;

    auto col_exclusions = get_col_exclusions(col);
    auto col_safe_iter = std::find_if(col_exclusions.begin(), col_exclusions.end(), is_state_excluded);
    if(col_safe_iter != col_exclusions.end())
        is_col_safe = false;

    auto sqr_exclusions = get_sqr_exclusions(row, col);
    auto sqr_safe_iter = std::find_if(sqr_exclusions.begin(), sqr_exclusions.end(), is_state_excluded);
    if(sqr_safe_iter != sqr_exclusions.end())
        is_block_safe = false;

    return is_row_safe && is_col_safe && is_block_safe;
}

/**
 * During de-collapse, we need to update the old block for all the blocks whose old was the block that's
 * being de-collapsed. Sometimes the block being de-collapsed is the same as the m_current_collapsed_block, and
 * we don't want a block to be itself's previous
 * @param old The block being de-collapsed
 * @return True if an exchange happened, false otherwise.
 */
void SudokuBoard::update_processing_chain(SudokuBlock *old)
{
    for(const auto& row : m_board)
    {
        for(const auto& block : row)
        {
            if(block->get_previous_block() == old)
            {
                block->set_previous_block(old->get_previous_block());
                old ->set_previous_block(m_current_collapsed);
            }
        }
    }
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

            if(value != 0)
                block->set_permanently_collapsed(true);
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
    m_initial_block = nullptr;
    s_stack_counter = 0;
    s_retries_count++;

    //TODO: Don't like having to read from file again, will need to find a way to save the initial state
    read_from_file(m_puzzle_file);
}

/**
 * We need the initial block with which we start to solve to not be one of the default_collapsed_blocks
 * @return A Non-default_collapsed first block with which we initiate the solve
 */
SudokuBlock *SudokuBoard::get_initial_block()
{
    std::vector<std::tuple<int, int>> coordinates_to_process;

    for(auto& row : m_board)
    {
        for(const auto& block : row)
        {
            if(!(block->get_is_permanently_collapsed()))
            {
                coordinates_to_process.emplace_back(block -> get_coordinate());
            }
        }
    }
    auto rand_index = generate_random_int(0, (int)coordinates_to_process.size() - 1);
    auto initial_coordinate = coordinates_to_process.at(rand_index);
    const auto& initial_block = m_board.at(std::get<0>(initial_coordinate)).at(std::get<1>(initial_coordinate));
    return initial_block.get();
}
