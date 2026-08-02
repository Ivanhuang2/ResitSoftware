/**     @file ModelPartList.h
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Template for model part list that will be used to create the trewview.
  *
  *     P Evans 2022
  */

#ifndef VIEWER_MODELPARTLIST_H
#define VIEWER_MODELPARTLIST_H


#include "ModelPart.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QList>

class ModelPart;

/** @class ModelPartList
  * @brief Qt item model that backs the treeview of model components.
  *
  * This is a standard QAbstractItemModel implementation over a tree of
  * ModelPart objects. The tree has a hidden root item (whose data supplies
  * the column headers); everything the user sees hangs below it. As with all
  * Qt item models the hidden root is addressed by an invalid QModelIndex.
  */
class ModelPartList : public QAbstractItemModel {
    Q_OBJECT        /**< A special Qt tag used to indicate that this is a special Qt class that might require preprocessing before compiling. */
public:

    /** Constructor
      * @param data is an identifying string for the list (not currently used for display)
      * @param parent is the Qt parent object that will own this model
      */
    ModelPartList( const QString& data, QObject* parent = NULL );


    /** Destructor - frees the whole tree of ModelParts via the root item */
    ~ModelPartList();

    /** Return column count
      * @param parent is the item whose columns are being counted (unused, all items have the same columns)
      * @return the number of columns in the tree (2 - "Part" and "Visible?")
      */
    int columnCount( const QModelIndex& parent ) const;

    /** This returns the value of a particular row (i.e. the item index) and
      *  columns (i.e. either the "Part" or "Visible" property).
      * @param index identifies the item and column being requested
      * @param role is what Qt intends to use the data for
      * @return the requested data, or an empty QVariant if not applicable
      */
    QVariant data( const QModelIndex& index, int role ) const;

    /** Standard function used by Qt internally.
      * @param index is the item being queried
      * @return the item flags for that item
      */
    Qt::ItemFlags flags( const QModelIndex& index ) const;


    /** Standard function used by Qt internally.
      * @param section is the column number
      * @param orientation is whether a row or column header is wanted
      * @param role is what Qt intends to use the data for
      * @return the header text, or an empty QVariant if not applicable
      */
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const;


    /** Get a valid QModelIndex for a location in the tree (row is the row in the tree under "parent"
      * or under the root of the tree if parent isnt specified. Column is either 0 = "Part" or 1 = "Visible"
      * in this example
      * @param row is the row below parent
      * @param column is the property column
      * @param parent is the parent item, or an invalid index for a top level item
      * @return a valid index, or an invalid index if row/column are out of range
      */
    QModelIndex index( int row, int column, const QModelIndex& parent ) const;


    /** Take a QModelIndex for an item, get a QModel Index for its parent
      * @param index is the item whose parent is wanted
      * @return index of the parent, or an invalid index if the item is top level
      */
    QModelIndex parent( const QModelIndex& index ) const;

    /** Get number of rows (items) under an item in tree
      * @param parent is the item being counted under, or an invalid index for the top level
      * @return number of child rows
      */
    int rowCount( const QModelIndex& parent ) const;

    /** Get a pointer to the root item of the tree
      * @return pointer to the hidden root item
      */
    ModelPart* getRootItem();

    /** Add a new item to the tree below parent.
      * @param parent is the item to add below; if it is an invalid index the
      *        item is added at the top level and parent is left invalid
      * @param data is the column data for the new item
      * @return a valid index referring to the newly created item
      */
    QModelIndex appendChild( QModelIndex& parent, const QList<QVariant>& data );

    /** Tell any attached views that the data held by an item has changed, so
      * that the treeview redraws that row. Used after the OptionDialog edits
      * a part's name or visibility.
      * @param index is the item that changed
      */
    void refreshItem( const QModelIndex& index );


private:
    ModelPart *rootItem;    /**< This is a pointer to the item at the base of the tree */
};
#endif

