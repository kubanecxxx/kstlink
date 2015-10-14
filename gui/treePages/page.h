#ifndef PAGE_H
#define PAGE_H

#include <QWidget>

class QTreeWidgetItem;
class Page : public QWidget
{
    Q_OBJECT
public:
    explicit Page(QWidget *parent = 0);
    
    //zobrazovat OK/cancel
    //save
    //load
    //cancel
    //...
    bool ShowButtonBox() {return showBbox;}
    QTreeWidgetItem * TreeItem() {return father;}
    void SetNewNamePointer(const QString * name){NewName = name;}
    void SetPrefix(const QString & prefix) {PagePrefix = prefix;}

signals:
    void Message(const QString & msg, int ms);
    
private slots:
    //virtual Accept()


protected:
    bool showBbox;

private:
     QTreeWidgetItem * father;
     void ReloadPageName();
     const QString * NewName;
     friend class WidgetTreePages;
     QString PagePrefix;
    
};

#endif // PAGE_H
