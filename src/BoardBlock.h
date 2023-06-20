//
// Created by User on 2023/06/10.
//

#ifndef WAVE_FUNCTION_COLLAPSE_BOARDBLOCK_H
#define WAVE_FUNCTION_COLLAPSE_BOARDBLOCK_H


#include <vector>
#include <memory>

class BoardBlock
{
public:
    BoardBlock();
    std::vector<int>& get_available_options();
    std::vector<int> get_remaining_options() const;
    int get_collapsed_value() const;
    void set_collapsed_value(int value);
    void set_coordinate(int x, int y);
    std::tuple<int,int> get_coordinate();
    void remove_option(int value);
    void set_current_block(bool is_current);
    bool operator< (const BoardBlock& other) const;
    bool operator== (const BoardBlock& other) const;
    unsigned int get_entropy() const;
    BoardBlock* get_previous() const;
    void set_previous(BoardBlock* previous);
    void add_available_option(int option);
    void set_available_options(const std::vector<int>& options);
private:
    BoardBlock* m_previous;
    bool m_current_block;
    int m_collapsed_value;
    std::tuple<int, int> m_coordinate;
    std::vector<int> m_available_options;
    std::vector<int> m_remaining_options;
};


#endif //WAVE_FUNCTION_COLLAPSE_BOARDBLOCK_H
