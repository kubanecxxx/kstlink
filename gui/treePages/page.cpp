#include "page.h"

Page::Page(QWidget *parent) :
    QWidget(parent),
    NewName(NULL)
{
    showBbox = false;
}

void Page::ReloadPageName()
{
    if (NewName)
        setWindowTitle(*NewName);
}
