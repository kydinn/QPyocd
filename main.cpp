/*
 * @Author       : kydin hikydin@gmail.com
 * @Date         : 2025-10-30 00:44:34
 * @LastEditors  : kydin hikydin@gmail.com
 * @LastEditTime : 2025-11-02 19:34:45
 * @FilePath     : \QPyocd\main.cpp
 * @Description  : 
 * Copyright (c) 2025 by kydin, email: hikydin@gmail.com, All Rights Reserved.
 */
#include "mcu_update_page.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    McuUpdatePage w;
    w.show();
    return a.exec();
}
