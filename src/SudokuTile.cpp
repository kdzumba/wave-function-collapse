//
// Created by Knowledge on 2023/06/08.
//

#include "SudokuTile.h"
#include <utility>

SudokuTile::SudokuTile(QString  value, int xPos, int yPos): m_value(std::move(value)), m_xPos(xPos), m_yPos(yPos)
{
}

void SudokuTile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter -> drawRoundedRect(m_xPos, m_yPos, TILE_SIZE, TILE_SIZE, 0, 0);
    painter ->drawText(m_xPos + TILE_SIZE / 2, m_yPos + TILE_SIZE / 2, m_value);
}

QRectF SudokuTile::boundingRect() const
{
    //Avoid repeating the return type, use brace initializers instead
    return {0, 0, TILE_SIZE, TILE_SIZE};
}
