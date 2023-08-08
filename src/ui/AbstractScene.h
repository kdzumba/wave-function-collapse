//
// Created by User on 2023/07/09.
//

#ifndef WAVE_FUNCTION_COLLAPSE_ABSTRACTSCENE_H
#define WAVE_FUNCTION_COLLAPSE_ABSTRACTSCENE_H

#include <QGraphicsScene>

class AbstractScene : public QGraphicsScene
{
    virtual void animate() = 0;
};
#endif //WAVE_FUNCTION_COLLAPSE_ABSTRACTSCENE_H
