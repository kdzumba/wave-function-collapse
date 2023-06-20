//
// Created by User on 2023/06/10.
//

#include "Board.h"
#include <random>
#include <algorithm>
#include <map>
#include <iostream>

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
        block->remove_option(value);
        auto current_row_options = block -> get_available_options();

        std::set_difference(block -> get_available_options().begin(),
                            block -> get_available_options().end(),
                            row_exclusions.begin(),
                            row_exclusions.end(),
                            current_row_options.begin());
        block ->set_available_options(current_row_options);
    }

    //remove start_value from available_options of col col_number
    auto col_exclusions = get_col_exclusions(col_number);
    for(auto& _row : m_board)
    {
        _row.at(col_number)->remove_option(value);
        auto current_col_options = _row.at(col_number) -> get_available_options();

        std::set_difference(_row.at(col_number) -> get_available_options().begin(),
                            _row.at(col_number) -> get_available_options().end(),
                            col_exclusions.begin(),
                            col_exclusions.end(),
                            current_col_options.begin());
        _row.at(col_number) ->set_available_options(current_col_options);
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

            std::set_difference(block -> get_available_options().begin(),
                                block -> get_available_options().end(),
                                sqr_exclusions.begin(),
                                sqr_exclusions.end(),
                                current_sqr_options.begin());
            block ->set_available_options(current_sqr_options);
        }
    }
}

const std::vector<std::vector<std::unique_ptr<BoardBlock>>>& Board::generate()
{
    s_stack_counter ++;
    //Collapse the next block with the least entropy
    auto next_block = least_entropy_block();

    //If there are no more available options for the next block, we've reached a dead end, and we need to
    //backtrack to a valid board configuration
    if(next_block -> get_entropy() == 0)
    {
        auto current = m_current_collapsed;

        //Reset all blocks that didn't have any remaining options at the time when they were set
        while(current->get_remaining_options().empty())
        {
            //Propagate de-collapse info then reset
            auto current_x = std::get<0>(current -> get_coordinate());
            auto current_y = std::get<1>(current -> get_coordinate());

            propagate_decollapse_info(current_x, current_y, current -> get_collapsed_value());
            current -> set_collapsed_value(0);
            current = current -> get_previous();
        }

        //Don't like the repetition here, but we need to de-collapse the last successfully placed block and remove
        //the last value it had from its available options
        auto current_x = std::get<0>(current -> get_coordinate());
        auto current_y = std::get<1>(current -> get_coordinate());

        propagate_decollapse_info(current_x, current_y, current -> get_collapsed_value());
        //If the option was re-added by the de-collapse, remove again
        current ->remove_option(current -> get_collapsed_value());

        next_block = current;
        if(next_block -> get_previous() != nullptr)
        {
            //The previous current_collapsed should be reset before reassigning the pointer
            //So that we don't have two blocks marked as current_blocks
            m_current_collapsed->set_current_block(false);
            m_current_collapsed = next_block->get_previous();
        }
        print_available_items();
    }

    //Choose next value from available options for next_block. We know it has at least 1 because otherwise we would backtrack
    auto rand_index = generate_random_int(0, (int) next_block -> get_available_options().size() - 1);
    auto next_value = next_block -> get_available_options().at(rand_index);
    next_block -> set_collapsed_value(next_value);
    next_block -> set_current_block(true);
    m_current_collapsed -> set_current_block(false);
    next_block -> set_previous(m_current_collapsed);

    m_current_collapsed = next_block;
    auto row_number = std::get<0>(next_block -> get_coordinate());
    auto col_number = std::get<1>(next_block -> get_coordinate());

    propagate_collapse_info(row_number, col_number, next_value);
    print();

    //Need to check if the block is full and terminate, otherwise generate()
    if(!is_fully_generated())
        generate();

//    print();
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
            if(block -> get_collapsed_value() == 0) //Only consider blocks that haven't been collapsed yet
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
    for(const auto& row : m_board)
    {
        for(const auto& block : row)
        {
            std::cout << block -> get_collapsed_value() << " ";
        }
        std::cout << std::endl;
    }
    printf("\n");
}

/**
 *
 * @param row_number The row number for which we want to compute the values that aren't available to blocks in that row
 * @return A vector of values that elements of row_number can't take
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
        block -> add_available_option(value);
    }

    //Every block in col should get value added back to their available options
    for(const auto& _row : m_board)
    {
        _row.at(col) ->add_available_option(value);
    }

    int start_row_index = row - row % MIN_FULL_BLOCK_SIZE;
    int start_col_index = col - col % MIN_FULL_BLOCK_SIZE;

    for(auto m = 0; m < MIN_FULL_BLOCK_SIZE; m++)
    {
        for(auto n = 0; n < MIN_FULL_BLOCK_SIZE; n++)
        {
            const auto& block = m_board.at(m + start_row_index).at(n + start_col_index);
            block ->add_available_option(value);
        }
    }
}

void Board::print_available_items()
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


