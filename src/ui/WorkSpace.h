//
// Created by User on 2023/07/09.
//

#ifndef WAVE_FUNCTION_COLLAPSE_WORKSPACE_H
#define WAVE_FUNCTION_COLLAPSE_WORKSPACE_H


#include <vector>
#include <QWidget>
#include <QGraphicsView>
#include "AbstractScene.h"

class WorkSpace : public QWidget
{
public:
    WorkSpace();
    QGraphicsView* get_graphics_view();
private:
    void init_default_scenes();
private:
    std::vector<std::unique_ptr<AbstractScene>> m_scene_states;
    QGraphicsView* m_graphics_view;
    static int s_current_state_index;
};


#endif //WAVE_FUNCTION_COLLAPSE_WORKSPACE_H
