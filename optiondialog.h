#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>

#include <ModelPart.h>

namespace Ui {
class OptionDialog;
}

class OptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionDialog(QWidget *parent = nullptr, ModelPart *part = nullptr );
    ~OptionDialog();

    void updatePartFromDialog(ModelPart* part);
    void updateDialogFromPart(ModelPart* part);

private:
    Ui::OptionDialog *ui;
    QColor colour;
    bool isVisible;
};

#endif // OPTIONDIALOG_H
