#include "optiondialog.h"
#include "ui_optiondialog.h"

#include <QLineEdit>

OptionDialog::OptionDialog(QWidget *parent, ModelPart *part ) :
    QDialog(parent),
    ui(new Ui::OptionDialog)
{
    ui->setupUi(this);


}

OptionDialog::~OptionDialog()
{
    delete ui;
}

void OptionDialog::updatePartFromDialog(ModelPart* part) {
    part->setVisible(ui->optionDia_visible->isChecked() );
    part->setColour(ui->optionDia_colourR->value(), ui->optionDia_colourG->value(), ui->optionDia_colourB->value());
    part->set(0, QVariant(ui->optionDia_name->text()) );
}


void OptionDialog::updateDialogFromPart(ModelPart* part) {
    ui->optionDia_visible->setChecked(part->visible());
    ui->optionDia_colourR->setValue(part->getColourR() );
    ui->optionDia_colourG->setValue(part->getColourG());
    ui->optionDia_colourB->setValue(part->getColourB());
    ui->optionDia_name->setText(part->data(0).toString());

}