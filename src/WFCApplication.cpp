//
// Created by Knowledge on 2023/06/08.
//

#include "WFCApplication.h"

WFCApplication::WFCApplication()
{
    m_window = new QMainWindow();
    m_window ->resize(1200, 780);
}

WFCApplication::~WFCApplication()
{
    delete m_window;
}

QMainWindow *WFCApplication::get_application_window()
{
    return m_window;
}
