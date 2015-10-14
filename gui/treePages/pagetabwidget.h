#ifndef PAGETABWIDGET_H
#define PAGETABWIDGET_H

#include <QWidget>
#include "page.h"

class QTabWidget;
class QTreeWidgetItem;

namespace Ui {
class PageTabWidget;
}

class PageTabWidget : public Page
{
    Q_OBJECT
    
public:
    explicit PageTabWidget(QWidget *parent = 0);
    ~PageTabWidget();
    void AddTab(Page * page, bool showIcon = true);

    
private:
    Ui::PageTabWidget *ui;
    friend class WidgetTreePages;

};

#endif // PAGETABWIDGET_H
