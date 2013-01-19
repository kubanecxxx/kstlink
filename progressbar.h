#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>
#include <QDialog>


namespace Ui {
class ProgressBar;
}

class ProgressBar : public QDialog
{
    Q_OBJECT
    
public:
    explicit ProgressBar(QWidget *parent = 0);
    ~ProgressBar();

public slots:
    void SetPercent(int perc);
    void SetAction(const QString & action);

private:
    Ui::ProgressBar *ui;
};

#endif // PROGRESSBAR_H
