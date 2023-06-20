////
//// Created by Knowledge on 2023/06/09.
////
//
//#ifndef WAVE_FUNCTION_COLLAPSE_SUDOKUBOARD_H
//#define WAVE_FUNCTION_COLLAPSE_SUDOKUBOARD_H
//
//
//#include <vector>
//#include "BoardBlock.h"
//
//class SudokuBoard
//{
//public:
//    explicit SudokuBoard(int number_of_clues);
//    bool isRowSafe(int value, int row_number);
//    bool isColSafe(int value, int col_number);
//    bool isBoxSafe(int value, int row_number, int col_number);
//    bool isSafeToPlace(int value, int row_number, int col_number);
//    [[nodiscard]] std::vector<std::vector<int>> getSudokuBoard() const;
//private:
//    static int generateRandomInt();
//    void generateNewBoard();
//    int generateValidEntry(int row_number, int col_number);
//private:
//    std::vector<std::vector<BoardBlock>> m_board;
//    bool m_valid = false;
//    int m_number_of_clues;
//    static int constexpr BOARD_SIZE = 9;
//};
//
//
//#endif //WAVE_FUNCTION_COLLAPSE_SUDOKUBOARD_H
