/**     @file optiondialog.cpp
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Dialog used to edit the properties (name, colour, visibility) of a
  *     single item in the model treeview.
  */

#include "optiondialog.h"
#include "ui_optiondialog.h"

#include <QLineEdit>
#include <QColorDialog>
#include <QSpinBox>
#include <QPushButton>

OptionDialog::OptionDialog(QWidget *parent, ModelPart *part ) :
    QDialog(parent),
    ui(new Ui::OptionDialog),
    colour(Qt::white)
{
    ui->setupUi(this);

    /* The "Choose Colour..." button opens the standard Qt colour chooser */
    connect(ui->optionDia_colourPicker, &QPushButton::clicked,
            this, &OptionDialog::chooseColour);

    /* Keep the swatch (and the cached QColour) in step with the spin boxes,
     * whichever way the user edits them */
    connect(ui->optionDia_colourR, &QSpinBox::valueChanged,
            this, &OptionDialog::updateColourPreview);
    connect(ui->optionDia_colourG, &QSpinBox::valueChanged,
            this, &OptionDialog::updateColourPreview);
    connect(ui->optionDia_colourB, &QSpinBox::valueChanged,
            this, &OptionDialog::updateColourPreview);

    if (part)
        updateDialogFromPart(part);
    else
        updateColourPreview();
}

OptionDialog::~OptionDialog()
{
    delete ui;
}

void OptionDialog::chooseColour()
{
    /* This is the Qt colour chooser required by the specification. It returns
     * an invalid QColor if the user presses Cancel, in which case the current
     * selection is left alone. */
    const QColor chosen = QColorDialog::getColor(colour, this, tr("Select Part Colour"));

    if (!chosen.isValid())
        return;

    ui->optionDia_colourR->setValue(chosen.red());
    ui->optionDia_colourG->setValue(chosen.green());
    ui->optionDia_colourB->setValue(chosen.blue());

    updateColourPreview();
}

void OptionDialog::updateColourPreview()
{
    colour = QColor(ui->optionDia_colourR->value(),
                    ui->optionDia_colourG->value(),
                    ui->optionDia_colourB->value());

    ui->optionDia_colourPreview->setStyleSheet(
        QStringLiteral("background-color: %1;").arg(colour.name()));
}

void OptionDialog::updatePartFromDialog(ModelPart* part) {
    if (!part)
        return;

    part->setVisible(ui->optionDia_visible->isChecked() );
    part->setColour(static_cast<unsigned char>(ui->optionDia_colourR->value()),
                    static_cast<unsigned char>(ui->optionDia_colourG->value()),
                    static_cast<unsigned char>(ui->optionDia_colourB->value()));
    part->set(0, QVariant(ui->optionDia_name->text()) );
}


void OptionDialog::updateDialogFromPart(ModelPart* part) {
    if (!part)
        return;

    ui->optionDia_visible->setChecked(part->visible());
    ui->optionDia_colourR->setValue(part->getColourR() );
    ui->optionDia_colourG->setValue(part->getColourG());
    ui->optionDia_colourB->setValue(part->getColourB());
    ui->optionDia_name->setText(part->data(0).toString());

    updateColourPreview();
}
