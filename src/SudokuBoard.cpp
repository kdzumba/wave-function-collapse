////
//// Created by Knowledge on 2023/06/09.
////
//
//#include "SudokuBoard.h"
//#include<random>
//#include <algorithm>
//
//SudokuBoard::SudokuBoard(int number_of_clues): m_number_of_clues(number_of_clues)
//{
//    for(auto n = 0; n < BOARD_SIZE; n++)
//    {
//        auto row = std::vector<int>(BOARD_SIZE, 0);
//        m_board.emplace_back(row);
//    }
//    generateNewBoard();
//}
//
////Generates a random number in the range [1, 9]
//int SudokuBoard::generateRandomInt()
//{
//    std::random_device rd;
//    std::mt19937 generator(rd());
//    std::uniform_int_distribution<int> distribution(1, 9);
//    return distribution(generator);
//}
//
////It's safe to put value in row at row_number if the row  doesn't already
////contain that value
//bool SudokuBoard::isRowSafe(int value, int row_number)
//{
//    std::vector<int> row = m_board.at(row_number);
//    return !(std::find(row.begin(), row.end(), value) != row.end());
//}
//
////It's safe to put a value in column at col_number if the column doesn't already
////contain that value.
//bool SudokuBoard::isColSafe(int value, int col_number)
//{
//    for(auto& row : m_board)
//    {
//        if(row.at(col_number) == value)
//            return false;
//    }
//    return true;
//}
//
//
////Every 3x3 block within the board should contain 9 unique numbers [1-9]
////It is safe to put a value in the block if the 3x3 block doesn't already
////contain that value
//bool SudokuBoard::isBoxSafe(int value, int row_number, int col_number)
//{
//    int start_row_index = row_number - row_number % 3;
//    int start_col_index = col_number - col_number % 3;
//    bool safe = true;
//
//    for(auto n = 0; n < 3; n++)
//    {
//        for(auto m = 0; m < 3; m++)
//        {
//            if(m_board.at(n + start_row_index).at(m + start_col_index) == value)
//                safe = false;
//        }
//    }
//    return safe;
//}
//
////It is safe to place <value> at a block if it satisfies the row, column and 3x3 grid constraints
//bool SudokuBoard::isSafeToPlace(int value, int row_number, int col_number)
//{
//    return isRowSafe(value, row_number) && isColSafe(value, col_number) && isBoxSafe(value, row_number, col_number);
//}
//
//std::vector<std::vector<int>> SudokuBoard::getSudokuBoard() const
//{
//    return m_board;
//}
//
//void SudokuBoard::generateNewBoard()
//{
//    for(auto row = 0; row < BOARD_SIZE; row++)
//    {
//        auto current_row = m_board.at(row);
//        for(auto col = 0; col < BOARD_SIZE; col++)
//        {
//            int value = generateValidEntry(row, col);
//            m_board.at(row).at(col) = value;
//        }
//    }
//}
//
//int SudokuBoard::generateValidEntry(int row_number, int col_number)
//{
//    int value = generateRandomInt();
//    if(isSafeToPlace(value, row_number, col_number))
//        return value;
//    else
//        value = generateValidEntry(row_number, col_number);
//    return value;
//}
