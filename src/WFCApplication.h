//
// Created by Knowledge Dzumba on 2023/06/08.
//

#ifndef WAVE_FUNCTION_COLLAPSE_WFCAPPLICATION_H
#define WAVE_FUNCTION_COLLAPSE_WFCAPPLICATION_H


#include <QObject>
#include <QMainWindow>

class WFCApplication : QObject
{
    Q_OBJECT
public:
    WFCApplication();
    ~WFCApplication() override;
    QMainWindow* get_application_window();
private:
    QMainWindow* m_window;

};


#endif //WAVE_FUNCTION_COLLAPSE_WFCAPPLICATION_H
