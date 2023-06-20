//
// Created by Knowledge Dzumba on 2023/06/08.
//

#ifndef WAVE_FUNCTION_COLLAPSE_MAINAPPLICATIONWINDOW_H
#define WAVE_FUNCTION_COLLAPSE_MAINAPPLICATIONWINDOW_H


#include <QObject>
#include <QMainWindow>

class MainApplicationWindow : QObject
{
    Q_OBJECT
public:
    MainApplicationWindow();
    ~MainApplicationWindow() override;
    QMainWindow* GetApplicationWindow();
private:
    QMainWindow* window;
};


#endif //WAVE_FUNCTION_COLLAPSE_MAINAPPLICATIONWINDOW_H
