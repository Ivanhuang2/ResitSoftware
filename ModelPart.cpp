/**     @file ModelPart.cpp
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Template for model parts that will be added as treeview items
  *
  *     P Evans 2022
  */

#include "ModelPart.h"

#include <vtkSmartPointer.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>


ModelPart::ModelPart(const QList<QVariant>& data, ModelPart* parent )
    : m_itemData(data), m_parentItem(parent), isVisible(true), colour(200, 200, 200) {

    /* Parts start off visible and a neutral light grey - the user can change
     * both of these from the OptionDialog. */
}


ModelPart::~ModelPart() {
    qDeleteAll(m_childItems);
}


void ModelPart::appendChild( ModelPart* item ) {
    /* Add another model part as a child of this part
     * (it will appear as a sub-branch in the treeview)
     */
    item->m_parentItem = this;
    m_childItems.append(item);
}


ModelPart* ModelPart::child( int row ) {
    /* Return pointer to child item in row below this item.
     */
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int ModelPart::childCount() const {
    /* Count number of child items
     */
    return m_childItems.count();
}


int ModelPart::columnCount() const {
    /* Count number of columns (properties) that this item has.
     */
    return m_itemData.count();
}

QVariant ModelPart::data(int column) const {
    /* Return the data associated with a column of this item
     *  Note on the QVariant type - it is a generic placeholder type
     *  that can take on the type of most Qt classes. It allows each
     *  column or property to store data of an arbitrary type.
     */
    if (column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}


void ModelPart::set(int column, const QVariant &value) {
    /* Set the data associated with a column of this item
     */
    if (column < 0 || column >= m_itemData.size())
        return;

    m_itemData.replace(column, value);
}


ModelPart* ModelPart::parentItem() {
    return m_parentItem;
}


int ModelPart::row() const {
    /* Return the row index of this item, relative to it's parent.
     */
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ModelPart*>(this));
    return 0;
}

void ModelPart::setColour(const unsigned char R, const unsigned char G, const unsigned char B) {
    colour[0] = R;
    colour[1] = G;
    colour[2] = B;

    /* Branch items (folders) have no actor, so only push the colour down to
     * VTK when this part actually has geometry. VTK works in 0-1 doubles
     * rather than 0-255 bytes, hence the division. */
    if (actor) {
        actor->GetProperty()->SetColor(R / 255.0, G / 255.0, B / 255.0);
    }
}

unsigned char ModelPart::getColourR() {
    return colour[0];
}

unsigned char ModelPart::getColourG() {
    return colour[1];
}


unsigned char ModelPart::getColourB() {
    return colour[2];
}


void ModelPart::setVisible(bool visible) {
    isVisible = visible;

    /* Column 1 of the treeview shows the visibility as text, keep it in sync */
    if (m_itemData.size() > 1) {
        m_itemData.replace(1, QVariant(visible ? QStringLiteral("true")
                                               : QStringLiteral("false")));
    }

    if (actor) {
        actor->SetVisibility(visible ? 1 : 0);
    }
}

bool ModelPart::visible() {
    return isVisible;
}

bool ModelPart::loadSTL( QString fileName ) {
    if (fileName.isEmpty())
        return false;

    /* 1. Use the vtkSTLReader class to load the STL file
     *     https://vtk.org/doc/nightly/html/classvtkSTLReader.html
     */
    file = vtkSmartPointer<vtkSTLReader>::New();
    file->SetFileName(fileName.toStdString().c_str());
    file->Update();

    /* A file that does not exist, or is not valid STL, still gives a reader
     * object - but the output will be empty. Detect that here so the caller
     * can report the failure rather than silently adding an invisible part. */
    if (file->GetOutput() == nullptr || file->GetOutput()->GetNumberOfCells() == 0) {
        file = nullptr;
        return false;
    }

    /* 2. Initialise the part's vtkMapper */
    mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputConnection(file->GetOutputPort());

    /* 3. Initialise the part's vtkActor and link to the mapper */
    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    /* Apply whatever colour / visibility this part is currently set to */
    actor->GetProperty()->SetColor(colour[0] / 255.0,
                                   colour[1] / 255.0,
                                   colour[2] / 255.0);
    actor->SetVisibility(isVisible ? 1 : 0);

    return true;
}

vtkSmartPointer<vtkActor> ModelPart::getActor() {
    return actor;
}

vtkActor* ModelPart::getNewActor() {
    /* The default mapper/actor combination can only be used to render the part in
     * the GUI, it CANNOT also be used to render the part in VR. This means you need
     * to create a second mapper/actor combination for use in VR - that is the role
     * of this function. */

    if (!file)
        return nullptr;

    /* 1. Create new mapper, fed from the same reader as the GUI actor */
    vtkSmartPointer<vtkDataSetMapper> newMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    newMapper->SetInputConnection(file->GetOutputPort());

    /* 2. Create new actor and link to mapper.
     *    A raw pointer is returned so that ownership passes to the caller
     *    (the VR thread's actor collection). */
    vtkActor* newActor = vtkActor::New();
    newActor->SetMapper(newMapper);

    /* 3. Copy the vtkProperties of the original actor to the new actor, so the
     *    colour the user picked in the GUI is carried across into VR.
     *
     *    See the vtkActor documentation, particularly the GetProperty() and SetProperty()
     *    functions.
     */
    if (actor) {
        newActor->GetProperty()->DeepCopy(actor->GetProperty());
        newActor->SetVisibility(actor->GetVisibility());
    }

    return newActor;
}
