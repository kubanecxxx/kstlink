#include "pagetabwidget.h"
#include "ui_pagetabwidget.h"
#include <QIcon>

PageTabWidget::PageTabWidget(QWidget *parent) :
    Page(parent),
    ui(new Ui::PageTabWidget)
{
    ui->setupUi(this);
}

PageTabWidget::~PageTabWidget()
{
    delete ui;
}

void PageTabWidget::AddTab(Page *page, bool showIcon)
{
    if (showIcon)
        ui->s->addTab(page,page->windowIcon(),page->windowTitle());
    else
        ui->s->addTab(page,page->windowTitle());
}
