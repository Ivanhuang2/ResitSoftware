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

class ModelPartList : public QAbstractItemModel {
    Q_OBJECT        /**< A special Qt tag used to indicate that this is a special Qt class that might require preprocessing before compiling. */
public:

    ModelPartList( const QString& data, QObject* parent = NULL );


    ~ModelPartList();

    /* Return column count */
    int columnCount( const QModelIndex& parent ) const;

    /* This returns the value of a particular row (i.e. the item index) and 
      *  columns (i.e. either the "Part" or "Visible" property).*/
    QVariant data( const QModelIndex& index, int role ) const;

    /* Standard function used by Qt internally.*/
    Qt::ItemFlags flags( const QModelIndex& index ) const;


    /* Standard function used by Qt internally.*/
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const;


    /* Get a valid QModelIndex for a location in the tree (row is the row in the tree under "parent"
      * or under the root of the tree if parent isnt specified. Column is either 0 = "Part" or 1 = "Visible" 
      * in this example */
    QModelIndex index( int row, int column, const QModelIndex& parent ) const;


    /* Take a QModelIndex for an item, get a QModel Index for its parent*/
    QModelIndex parent( const QModelIndex& index ) const;

    /* Get number of rows (items) under an item in tree */
    int rowCount( const QModelIndex& parent ) const;

    /* Get a pointer to the root item of the tree */
    ModelPart* getRootItem();

    QModelIndex appendChild( QModelIndex& parent, const QList<QVariant>& data );


private:
    ModelPart *rootItem;    /**< This is a pointer to the item at the base of the tree */
};
#endif

