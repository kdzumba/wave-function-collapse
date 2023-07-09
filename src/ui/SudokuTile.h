//
// Created by Knowledge on 2023/06/08.
//

#ifndef WAVE_FUNCTION_COLLAPSE_SUDOKUTILE_H
#define WAVE_FUNCTION_COLLAPSE_SUDOKUTILE_H


#include <QWidget>
#include <QGraphicsItem>
#include <QPainter>
#include "../models/BoardBlock.h"

class SudokuTile : public QGraphicsItem
{
public:
    explicit SudokuTile(QString  value, int x, int y);
    explicit SudokuTile(BoardBlock* block);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
private:
    QString m_value;
    bool m_border_top;
    bool m_border_bottom;
    bool m_border_left;
    bool m_border_right;
    int m_x;
    int m_y;
    BoardBlock* m_block;
public:
    static constexpr int TILE_SIZE = 90;
};


#endif //WAVE_FUNCTION_COLLAPSE_SUDOKUTILE_H
