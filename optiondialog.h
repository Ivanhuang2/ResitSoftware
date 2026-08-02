/**     @file optiondialog.h
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Dialog used to edit the properties (name, colour, visibility) of a
  *     single item in the model treeview.
  */

#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>
#include <QColor>

#include <ModelPart.h>

namespace Ui {
class OptionDialog;
}

/** @class OptionDialog
  * @brief Property editor for one ModelPart.
  *
  * Opened when the user right-clicks a treeview item (or picks Model > Item
  * Options). Offers the part name, a visibility checkbox and the part colour.
  *
  * The colour can be set either with the three R/G/B spin boxes or through the
  * standard Qt colour chooser (QColorDialog), which is launched by the
  * "Choose Colour..." button. Both routes stay in sync, and the spin boxes
  * remain the value that is written back to the ModelPart.
  */
class OptionDialog : public QDialog
{
    Q_OBJECT

public:
    /** Constructor
      * @param parent is the widget that owns this dialog
      * @param part is the ModelPart whose current properties should be shown;
      *        may be nullptr, in which case the dialog opens with defaults
      */
    explicit OptionDialog(QWidget *parent = nullptr, ModelPart *part = nullptr );

    /** Destructor - frees the generated UI object */
    ~OptionDialog();

    /** Copy the values currently shown in the dialog onto a model part.
      * @param part is the part to write to (ignored if nullptr)
      */
    void updatePartFromDialog(ModelPart* part);

    /** Load the current properties of a model part into the dialog widgets.
      * @param part is the part to read from (ignored if nullptr)
      */
    void updateDialogFromPart(ModelPart* part);

private slots:
    /** Launch the Qt colour chooser and copy the chosen colour into the
      * R/G/B spin boxes. Does nothing if the user cancels the chooser. */
    void chooseColour();

    /** Rebuild the cached colour from the spin boxes and repaint the small
      * colour swatch next to them. Called whenever a spin box changes. */
    void updateColourPreview();

private:
    Ui::OptionDialog *ui;       /**< Pointer to the widgets generated from optiondialog.ui */
    QColor colour;              /**< Colour currently shown in the dialog, mirrors the R/G/B spin boxes */
};

#endif // OPTIONDIALOG_H
