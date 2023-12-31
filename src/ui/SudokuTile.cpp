//
// Created by Knowledge on 2023/06/08.
//

#include "SudokuTile.h"
#include <utility>

SudokuTile::SudokuTile(QString  value, int x, int y): m_value(std::move(value)), m_x(x), m_y(y)
{
    if(m_x % 3 == 0)
        m_border_left = true;
    if(m_y % 3 == 0)
        m_border_top = true;
    if(m_x == 8)
        m_border_right = true;
    if(m_y == 8)
        m_border_bottom = true;
}

void SudokuTile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter -> drawRoundedRect(m_x, m_y, TILE_SIZE, TILE_SIZE, 0, 0);
    painter -> setFont(QFont("Arial", 30));

    if(m_value.toInt() != 0)
        painter -> drawText(m_x + TILE_SIZE / 2, m_y + TILE_SIZE / 2, m_value);
    else painter ->setBackground(QBrush(QColor(100, 100, 50)));
}

QRectF SudokuTile::boundingRect() const
{
    //Avoid repeating the return type, use brace initializers instead
    return {0, 0, TILE_SIZE, TILE_SIZE};
}

SudokuTile::SudokuTile(SudokuBlock* block)
{
    m_block = block;
}
