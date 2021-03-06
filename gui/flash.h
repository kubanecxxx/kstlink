#ifndef FLASH_H
#define FLASH_H

#include <QWidget>
#include "page.h"

namespace Ui {
class Flash;
}

class QFile;
class Flash : public Page
{
    Q_OBJECT

public:
    explicit Flash(QWidget *parent = 0);
    ~Flash();

private slots:
    void on_buttonFile_clicked();
    void on_buttonFlash_clicked();
    void on_editFilename_textChanged(const QString &arg1);

public slots:
    void Erasing(int percent);
    void Verifing(int percent);
    void Flashing(int percent);
    void Success(bool ok);
    void EnableWidget(bool enable);
    void ErasingActive(bool active);

private:
    Ui::Flash *ui;
    QString lastDir;
    bool enabled;
    bool erasing;
    bool flashed;


signals:
    void flashEraseRequest();
    void flashWriteRequest(const QString & filename);
    void restartRequest();

};

#endif // FLASH_H
