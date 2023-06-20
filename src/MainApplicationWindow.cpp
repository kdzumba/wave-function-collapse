//
// Created by Knowledge on 2023/06/08.
//

#include "MainApplicationWindow.h"

MainApplicationWindow::MainApplicationWindow()
{
    window = new QMainWindow();
}

QMainWindow *MainApplicationWindow::GetApplicationWindow(){
    return this -> window;
}

MainApplicationWindow::~MainApplicationWindow()
{
    delete window;
}
