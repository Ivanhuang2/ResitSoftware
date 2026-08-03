/**     @file ModelPart.h
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Template for model parts that will be added as treeview items
  *
  *     P Evans 2022
  */

#ifndef VIEWER_MODELPART_H
#define VIEWER_MODELPART_H

#include <QString>
#include <QList>
#include <QVariant>

/* VTK headers */
#include <vtkSmartPointer.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkSTLReader.h>
#include <vtkColor.h>

/** @class ModelPart
  * @brief A single node in the model tree.
  *
  * Each ModelPart is one row of the treeview. A part may be a "branch"
  * (a folder loaded by Open Directory, which has children but no geometry)
  * or a "leaf" (a single STL file, which owns a vtkSTLReader / vtkDataSetMapper
  * / vtkActor chain used to draw it in the VTK view).
  *
  * The class also stores the per-part display properties that the user can
  * edit through the OptionDialog: the RGB colour and the visibility flag.
  */
class ModelPart {
public:
    /** Constructor
     * @param data is a List (array) of strings for each property of this item (part name and visiblity in our case
     * @param parent is the parent of this item (one level up in tree)
     */
    ModelPart(const QList<QVariant>& data, ModelPart* parent = nullptr);

    /** Destructor
      * Needs to free array of child items
      */
    ~ModelPart();

    /** Add a child to this item.
      * @param item Pointer to child object (must already be allocated using new)
      */
    void appendChild(ModelPart* item);

    /** Return child at position 'row' below this item
      * @param row is the row number (below this item)
      * @return pointer to the item requested, or nullptr if row is out of range
      */
    ModelPart* child(int row);

    /** Return number of children to this item
      * @return number of children
      */
    int childCount() const;         /* Note on the 'const' keyword - it means that this function is valid for
                                     * constant instances of this class. If a class is declared 'const' then it
                                     * cannot be modifed, this means that 'set' type functions are usually not
                                     * valid, but 'get' type functions are.
                                     */

    /** Get number of data items (2 - part name and visibility string) in this case.
      * @return number of visible data columns
      */
    int columnCount() const;

    /** Return the data item at a particular column for this item.
      * i.e. either part name of visibility
      * used by Qt when displaying tree
      * @param column is column index
      * @return the QVariant (represents string), or an empty QVariant if column is out of range
      */
    QVariant data(int column) const;


    /** Default function required by Qt to allow setting of part
      * properties within treeview.
      * @param column is the index of the property to set
      * @param value is the value to apply
      */
    void set( int column, const QVariant& value );

    /** Get pointer to parent item
      * @return pointer to parent item, or nullptr if this is the root
      */
    ModelPart* parentItem();

    /** Get row index of item, relative to parent item
      * @return row index
      */
    int row() const;


    /** Set colour of this part.
      * The colour is stored on the part and, if the part has already been
      * loaded from an STL file, applied immediately to its vtkActor so that
      * the change shows up in the VTK view.
      * @param R is the red channel   (0-255)
      * @param G is the green channel (0-255)
      * @param B is the blue channel  (0-255)
      */
    void setColour(const unsigned char R, const unsigned char G, const unsigned char B);

    /** Get the red channel of this part's colour.
      * @return red value in the range 0-255
      */
    unsigned char getColourR();

    /** Get the green channel of this part's colour.
      * @return green value in the range 0-255
      */
    unsigned char getColourG();

    /** Get the blue channel of this part's colour.
      * @return blue value in the range 0-255
      */
    unsigned char getColourB();

    /** Set visible flag
      * @param visible sets visible/non-visible
      */
    void setVisible(bool visible);

    /** Get visible flag
      * @return visible flag as boolean
      */
    bool visible();

	/** Load STL file
      * Builds the vtkSTLReader -> vtkDataSetMapper -> vtkActor pipeline for
      * this part and applies the currently stored colour and visibility.
      * @param fileName is the full path of the STL file to load
      * @return true if the file was read and contained geometry, false otherwise
      */
    bool loadSTL(QString fileName);

    /** Return actor
      * @return pointer to default actor for GUI rendering, or a null smart
      *         pointer if this part has no geometry (e.g. a folder branch)
      */
    vtkSmartPointer<vtkActor> getActor();

    /** Return new actor for use in VR
      * The actor returned by getActor() is already owned by the GUI renderer
      * and cannot be shared with the VR renderer, so this builds a second
      * mapper/actor pair over the same source data. The two actors share a
      * single vtkProperty, so a colour change made in the GUI is reflected in
      * the VR view as well.
      * @return smart pointer to the new actor, or a null smart pointer if this
      *         part has no geometry
      */
    vtkSmartPointer<vtkActor> getNewActor();

private:
    QList<ModelPart*>                           m_childItems;       /**< List (array) of child items */
    QList<QVariant>                             m_itemData;         /**< List (array of column data for item */
    ModelPart*                                  m_parentItem;       /**< Pointer to parent */

    bool                                        isVisible;          /**< True/false to indicate if should be visible in model rendering */

	vtkSmartPointer<vtkSTLReader>               file;               /**< Datafile from which part loaded */
    vtkSmartPointer<vtkDataSetMapper>           mapper;             /**< Mapper for rendering */
    vtkSmartPointer<vtkActor>                   actor;              /**< Actor for rendering */
    vtkColor3<unsigned char>                    colour;             /**< User defineable colour */
};


#endif

