//
// Created by User on 2023/08/04.
//

#ifndef WAVE_FUNCTION_COLLAPSE_TILE_H
#define WAVE_FUNCTION_COLLAPSE_TILE_H


class BlockState
{
public:
    explicit BlockState(int value);
    int get_value() const;
    bool operator==(const BlockState& other) const;
    bool operator!=(const BlockState& other) const;
private:
    int m_value;
};


#endif //WAVE_FUNCTION_COLLAPSE_TILE_H
