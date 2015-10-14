#include "widgettreepages.h"
#include "ui_widgettreepages.h"
#include "page.h"
#include <QStatusBar>
#include <QTableWidget>
#include "pagetabwidget.h"

WidgetTreePages::WidgetTreePages(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetTreePages),
    first(NULL),
    AutoSetParentTitle(true)
{
    ui->setupUi(this);
    ui->buttonBox->setShown(false);
    ui->line_2->setShown(false);
}

WidgetTreePages::~WidgetTreePages()
{
    delete ui;
}

void WidgetTreePages::on_list_itemClicked(QTreeWidgetItem *item, int )
{
    Page * page = PageList.value(item , NULL);

    if (page == NULL)
        return;

    bool jo = page->ShowButtonBox();

    QString name = page->PagePrefix + page->windowTitle();
    ui->labName->setText(name);
    ui->labIcon->setPixmap(page->windowIcon().pixmap(24,24));
    ui->stackedWidget->setCurrentWidget(page);

    ui->buttonBox->setShown(jo);
    ui->line_2->setShown(jo);

    setWindowTitle(page->windowTitle());
    setWindowIcon(page->windowIcon());
    emit ReloadTitle();

    if (parentWidget() && AutoSetParentTitle)
    {
        parentWidget()->setWindowTitle(page->windowTitle());
        parentWidget()->setWindowIcon(page->windowIcon());
    }
}

QTreeWidgetItem * WidgetTreePages::AddPage(Page * page, QTreeWidgetItem * parent , const QString & text)
{
    if (parent == NULL)
        parent = ui->list->invisibleRootItem();

    QTreeWidgetItem * item = new QTreeWidgetItem;
    //pokud page bude null tak přidat jenom stroma a nenechat na něho klikat
    if (page)
    {
        //přidat do stack widgetu
        item->setIcon(0,page->windowIcon());
        item->setText(0,page->windowTitle());
        PageList.insert(item,page);
        ui->stackedWidget->addWidget(page);
        //připojit signály z buttonboxu
        connect(page,SIGNAL(Message(QString,int)),this,SIGNAL(Message(QString,int)));

        page->father = item;
    }
    else
    {
        item->setFlags(Qt::ItemIsEnabled);
        item->setText(0,text);
    }

    parent->addChild(item);


    if (first == NULL)
    {
        first = item;
        ui->list->setItemSelected(first,true);
        on_list_itemClicked(first,0);

    }
    return item;
}

bool WidgetTreePages::connectStatusBar(QStatusBar *bar)
{
    return connect(this,SIGNAL(Message(QString,int)),bar,SLOT(showMessage(QString,int)));
}

PageTabWidget * WidgetTreePages::AddTabWidget(const QString &name, const QIcon &icon, QTreeWidgetItem * parent)
{
    PageTabWidget * page = new PageTabWidget(this);
    page->setWindowTitle(name);
    page->setWindowIcon(icon);
    AddPage(page,parent);

    return page;
}

void WidgetTreePages::InsertHeaderWidget(QWidget *widget)
{
    ui->horizontalLayout->insertWidget(0,widget);
}

void WidgetTreePages::Clear()
{
    first = NULL;

    foreach (Page* pag, PageList.values()) {
        pag->deleteLater();
    }

    ui->list->clear();
    PageList.clear();
}

void WidgetTreePages::ReloadPageNames()
{
    QMapIterator<QTreeWidgetItem*, Page*> it(PageList);

    while(it.hasNext())
    {
        it.next();

        it.value()->ReloadPageName();
        it.key()->setText(0,it.value()->windowTitle());
        if (ui->list->selectedItems().count())
            on_list_itemClicked(ui->list->selectedItems().at(0),0);
    }
}

void WidgetTreePages::ChangeSides()
{
    QLayoutItem * it = ui->horizontalLayout_2->takeAt(0);
    ui->horizontalLayout_2->addItem(it);
}
