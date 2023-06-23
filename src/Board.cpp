//
// Created by Kdzumba on 2023/06/10.
//

#include "Board.h"
#include <random>
#include <algorithm>
#include <map>
#include <iostream>
#include <iomanip>

int Board::s_stack_counter = 0;

Board::Board()
{
    for(auto x = 0; x < BOARD_SIZE; x++)
    {
        auto row = std::vector<std::unique_ptr<BoardBlock>>();
        for(auto y = 0; y < BOARD_SIZE; y++)
        {
            auto block = std::make_unique<BoardBlock>();
            block -> set_coordinate(x, y);
            row.emplace_back(std::move(block));
        }
        m_board.emplace_back(std::move(row));
    }
    m_current_collapsed = nullptr;
    init();
    generate();
}

void Board::init()
{
    auto start_coordinate = random_coordinate();
    int x = std::get<0>(start_coordinate);
    int y = std::get<1>(start_coordinate);
    int start_value = generate_random_int(1, 9);
    m_board.at(x).at(y) -> set_collapsed_value(start_value);

    //Setting the current_block ensures that it won't be considered for min entropy later on
    m_board.at(x).at(y) ->set_current_block(true);
    this -> m_current_collapsed = m_board.at(x).at(y).get();
    propagate_collapse_info(x, y, start_value);
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
void Board::propagate_collapse_info(int row_number, int col_number, int value)
{
    auto row_exclusions = get_row_exclusions(row_number);

    // Remove exclusions from available options of blocks at row_number
    for(const auto& block : m_board.at(row_number))
    {
        block -> remove_option(value);
        auto new_row_options = std::vector<int>();

        //We need options that haven't been excluded by the current generate, hence set_difference
        std::set_difference(block -> get_available_options().begin(), block -> get_available_options().end(),
                            row_exclusions.begin(), row_exclusions.end(), std::inserter(new_row_options, new_row_options.begin()));
        block ->set_available_options(new_row_options);
    }

    //remove start_value from available_options of col col_number
    auto col_exclusions = get_col_exclusions(col_number);
    for(auto& _row : m_board)
    {
        _row.at(col_number)->remove_option(value);
        auto current_col_options = _row.at(col_number) -> get_available_options();
        const auto& block = _row.at(col_number);

        std::set_difference(block -> get_available_options().begin(),block -> get_available_options().end(),
                            col_exclusions.begin(),col_exclusions.end(),current_col_options.begin());
        block ->set_available_options(current_col_options);
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
            block->remove_option(value);
            auto current_sqr_options = block -> get_available_options();

            std::set_difference(block -> get_available_options().begin(),block -> get_available_options().end(),
                                sqr_exclusions.begin(),sqr_exclusions.end(),current_sqr_options.begin());
            block ->set_available_options(current_sqr_options);
        }
    }
}

void Board::collapse(BoardBlock* block)
{
    auto rand_index = generate_random_int(0, (int) block -> get_available_options().size() - 1);
    auto next_value = block -> get_available_options().at(rand_index);
    block -> set_collapsed_value(next_value);
    block -> set_current_block(true);
    m_current_collapsed -> set_current_block(false);
    block -> set_previous(m_current_collapsed);

    m_current_collapsed = block;
    auto row_number = std::get<0>(block -> get_coordinate());
    auto col_number = std::get<1>(block -> get_coordinate());

    propagate_collapse_info(row_number, col_number, next_value);
}


BoardBlock* Board::backtrack()
{
    auto current = m_current_collapsed;

    while(current -> get_available_options().empty())
    {
        current = current -> get_previous();
    }

    std::cout << "Backtracking" << std::endl;
    auto current_value = current -> get_collapsed_value();
    current -> set_collapsed_value(0);
    propagate_decollapse_info(std::get<0>(current -> get_coordinate()), std::get<1>(current -> get_coordinate()), current_value);

    return current;
}

const std::vector<std::vector<std::unique_ptr<BoardBlock>>>& Board::generate()
{
    s_stack_counter ++;
    auto next_block = least_entropy_block();

    if(next_block -> get_entropy() == 0)
        next_block = backtrack();

    collapse(next_block);

    print();

    if(!is_fully_generated() && s_stack_counter < MAX_GENERATE_RETRIES)
        generate();

    return m_board;
}


std::tuple<int, int> Board::random_coordinate() {
    int x = generate_random_int(0, 8);
    int y = generate_random_int(0, 8);
    return std::make_tuple(x, y);
}

int Board::generate_random_int(int start, int end) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(start, end);
    return distribution(generator);
}

BoardBlock* Board::least_entropy_block()
{
    std::map<unsigned int, std::vector<BoardBlock*>> entropy_to_block_map;
    for(const auto& row : m_board)
    {
        for(const auto & block : row)
        {
            //Only consider blocks that haven't been collapsed yet
            if(block-> get_collapsed_value() == 0)
            {
                unsigned int key = block -> get_entropy();
                if(entropy_to_block_map.find(key) != entropy_to_block_map.end() && key != 0)
                {
                    entropy_to_block_map.at(key).emplace_back(block.get());
                }
                else
                {
                    auto value = std::vector<BoardBlock*>{block.get()};
                    entropy_to_block_map.insert({key, value});
                }
            }
        }
    }

    auto min_entropy = entropy_to_block_map.begin() -> second;
    auto rand_index = generate_random_int(0, (int)(min_entropy.size() - 1));
    return min_entropy.at(rand_index);
}

bool Board::is_fully_generated() const
{
    bool is_full = true;
    for(const auto& row : m_board)
    {
        for(const auto & col : row)
        {
            if(col -> get_collapsed_value() == 0)
                is_full = false;
        }
    }
    return is_full;
}


void Board::print()
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
            std::cout << block -> get_collapsed_value() << " ";
        }
        row_count++;
        std::cout << std::endl;
    }
    printf("\n");
}

/**
 * When a collapse/de-generate happens, we need to re-compute the values that aren't available to blocks in the same row
 * as the collapsed/de-collapsed block
 * @param row_number The row number for which we want to compute the values that aren't available to blocks in that row
 * @return A vector of values that blocks in row_number can't take
 */

std::vector<int> Board::get_row_exclusions(int row_number)
{
    auto row_exclusions = std::vector<int>();

    for(const auto& block : m_board.at(row_number))
    {
        if(block -> get_collapsed_value() != 0)
            row_exclusions.emplace_back(block -> get_collapsed_value());
    }

    std::sort(row_exclusions.begin(), row_exclusions.end());
    return row_exclusions;
}

/**
 * When a collapse/de-generate happens, we need to re-compute the values that aren't available to blocks in the same row
 * as the collapsed/de-collapsed block
 * @param col_number The col number for which we want to compute the values that aren't available to blocks in that col
 * @return A vector of values that blocks in col_number can't take
 */
std::vector<int> Board::get_col_exclusions(int col_number)
{
    auto col_exclusions = std::vector<int>();

    for(auto& _row : m_board)
    {
        if(_row.at(col_number) -> get_collapsed_value() != 0)
            col_exclusions.emplace_back(_row.at(col_number) -> get_collapsed_value());
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
std::vector<int> Board::get_sqr_exclusions(int row_number, int col_number)
{
    auto sqr_exclusions = std::vector<int>();

    int start_row_index = row_number - row_number % MIN_FULL_BLOCK_SIZE;
    int start_col_index = col_number - col_number % MIN_FULL_BLOCK_SIZE;

    for(auto m = 0; m < MIN_FULL_BLOCK_SIZE; m++)
    {
        for (auto n = 0; n < MIN_FULL_BLOCK_SIZE; n++)
        {
            const auto& block = m_board.at(m + start_row_index).at(n + start_col_index);
            if(block -> get_collapsed_value() != 0)
                sqr_exclusions.emplace_back(block -> get_collapsed_value());
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
void Board::propagate_decollapse_info(int row, int col, int value)
{
    //Every block in row should get value added to their available options
    for(const auto& block : m_board.at(row))
    {
        auto current_row = std::get<0>(block -> get_coordinate());
        auto current_col = std::get<1>(block -> get_coordinate());

        if(!(current_row == row && current_col == col) && is_safe(current_row, current_col, value))
            block -> add_available_option(value);
    }

    //Every block in col should get value added back to their available options
    for(const auto& _row : m_board)
    {
        const auto& block = _row.at(col);
        auto current_row = std::get<0>(block -> get_coordinate());
        auto current_col = std::get<1>(block -> get_coordinate());

        if(!(current_row == row && current_col == col) && is_safe(current_row, current_col, value))
            block -> add_available_option(value);
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
                block -> add_available_option(value);
        }
    }
}


/**
 * Only used for debugging to see available options for blocks at a certain point in the code
 */
void Board::print_available_options()
{
    for(const auto& row : m_board)
    {
        for(const auto& block : row)
        {
            auto current_x = std::get<0>(block -> get_coordinate());
            auto current_y = std::get<1>(block -> get_coordinate());
            std::cout << "(" << current_x << " , " << current_y << ")" << " --> " << "{";
            for(auto item : block -> get_available_options())
                std::cout << item << " ";
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
bool Board::is_safe(int row, int col, int value)
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


