//
// Created by Knowledge on 2023/06/08.
//

#ifndef WAVE_FUNCTION_COLLAPSE_SUDOKUTILE_H
#define WAVE_FUNCTION_COLLAPSE_SUDOKUTILE_H


#include <QWidget>
#include <QGraphicsItem>
#include <QPainter>

class SudokuTile : public QGraphicsItem
{
public:
    explicit SudokuTile(QString  value, int xPos, int yPos);
    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
private:
    QString m_value;
    int m_xPos;
    int m_yPos;
public:
    static constexpr int TILE_SIZE = 50;
};


#endif //WAVE_FUNCTION_COLLAPSE_SUDOKUTILE_H
