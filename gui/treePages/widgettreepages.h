#ifndef WIDGETTREEPAGES_H
#define WIDGETTREEPAGES_H

#include <QWidget>
#include <QMap>

namespace Ui {
class WidgetTreePages;
}

class QStatusBar;
class Page;
class PageTabWidget;
class QTreeWidgetItem;
class QTabWidget;
class WidgetTreePages : public QWidget
{
    Q_OBJECT
    
public:
    explicit WidgetTreePages(QWidget *parent = 0);
    ~WidgetTreePages();
    
    QTreeWidgetItem * AddPage(Page * page, QTreeWidgetItem * parent = NULL, const QString & text = QString());
    PageTabWidget * AddTabWidget(const QString & name, const QIcon & icon, QTreeWidgetItem * parent = NULL);
    bool connectStatusBar(QStatusBar * bar);
    void InsertHeaderWidget(QWidget* widget);
    void Clear();
    void SetAutoSetParentTitle(bool enabled) {AutoSetParentTitle = enabled;}
    void ChangeSides();

public slots:
    void on_list_itemClicked(QTreeWidgetItem *item, int column);
    void ReloadPageNames();

private:
    Ui::WidgetTreePages *ui;
    QMap<QTreeWidgetItem*, Page*> PageList;
    QTreeWidgetItem * first;
    bool AutoSetParentTitle;

signals:
    void Message(const QString & msg, int ms);
    void ReloadTitle();
};

#endif // WIDGETTREEPAGES_H
