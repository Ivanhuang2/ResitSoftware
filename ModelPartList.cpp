/**     @file ModelPartList.cpp
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Template for model part list that will be used to create the trewview.
  *
  *     P Evans 2022
  */

#include "ModelPartList.h"
#include "ModelPart.h"

ModelPartList::ModelPartList( const QString& data, QObject* parent ) : QAbstractItemModel(parent) {
    /* Have option to specify number of visible properties for each item in tree - the root item
     * acts as the column headers
     */
    Q_UNUSED(data);

    rootItem = new ModelPart( { tr("Part"), tr("Visible?") } );
}



ModelPartList::~ModelPartList() {
    delete rootItem;
}


int ModelPartList::columnCount( const QModelIndex& parent ) const {
    Q_UNUSED(parent);

    return rootItem->columnCount();
}


QVariant ModelPartList::data( const QModelIndex& index, int role ) const {
    /* If the item index isnt valid, return a new, empty QVariant (QVariant is generic datatype
     * that could be any valid QT class) */
    if( !index.isValid() )
        return QVariant();

    /* Role represents what this data will be used for, we only need deal with the case
     * when QT is asking for data to create and display the treeview. Return a new,
     * empty QVariant if any other request comes through. */
    if (role != Qt::DisplayRole)
        return QVariant();

    /* Get a a pointer to the item referred to by the QModelIndex */
    ModelPart* item = static_cast<ModelPart*>( index.internalPointer() );
    if( !item )
        return QVariant();

    /* Each item in the tree has a number of columns ("Part" and "Visible" in this
     * initial example) return the column requested by the QModelIndex */
    return item->data( index.column() );
}


Qt::ItemFlags ModelPartList::flags( const QModelIndex& index ) const {
    if( !index.isValid() )
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags( index );
}


QVariant ModelPartList::headerData( int section, Qt::Orientation orientation, int role ) const {
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
        return rootItem->data( section );

    return QVariant();
}


QModelIndex ModelPartList::index(int row, int column, const QModelIndex& parent) const {
    /* hasIndex() checks row/column are within range for this parent. If they
     * are not there is no such item, so an invalid index must be returned.
     *
     * Note: the original template fell back to the root item when hasIndex()
     * failed, which meant asking a leaf item for a child handed back the first
     * top level item instead of an empty index. */
    if( !hasIndex(row, column, parent) )
        return QModelIndex();

    ModelPart* parentItem;

    if( !parent.isValid() )
        parentItem = rootItem;              // an invalid parent means the hidden root
    else
        parentItem = static_cast<ModelPart*>(parent.internalPointer());

    if( !parentItem )
        return QModelIndex();

    ModelPart* childItem = parentItem->child(row);
    if( childItem )
        return createIndex(row, column, childItem);


    return QModelIndex();
}


QModelIndex ModelPartList::parent( const QModelIndex& index ) const {
    if (!index.isValid())
        return QModelIndex();

    ModelPart* childItem = static_cast<ModelPart*>(index.internalPointer());
    if( !childItem )
        return QModelIndex();

    ModelPart* parentItem = childItem->parentItem();

    /* The hidden root is represented by an invalid index. The nullptr check
     * also covers the case where index refers to the root item itself -
     * without it, parentItem->row() below would dereference a null pointer. */
    if( parentItem == rootItem || parentItem == nullptr )
        return QModelIndex();

    return createIndex( parentItem->row(), 0, parentItem );
}


int ModelPartList::rowCount( const QModelIndex& parent ) const {
    ModelPart* parentItem;
    if( parent.column() > 0 )
        return 0;

    if( !parent.isValid() )
        parentItem = rootItem;
    else
        parentItem = static_cast<ModelPart*>(parent.internalPointer());

    if( !parentItem )
        return 0;

    return parentItem->childCount();
}


ModelPart* ModelPartList::getRootItem() {
    return rootItem;
}



QModelIndex ModelPartList::appendChild(QModelIndex& parent, const QList<QVariant>& data) {
    ModelPart* parentPart;

    if (parent.isValid()) {
        parentPart = static_cast<ModelPart*>(parent.internalPointer());
    }
    else {
        /* Qt represents the hidden root item with an INVALID index. The
         * original template built an index wrapping rootItem here, which then
         * made parent() dereference a null pointer during beginInsertRows(). */
        parentPart = rootItem;
        parent = QModelIndex();
    }

    if (!parentPart)
        return QModelIndex();

    const int newRow = parentPart->childCount();

    beginInsertRows( parent, newRow, newRow );

    ModelPart* childPart = new ModelPart( data, parentPart );

    parentPart->appendChild(childPart);

    endInsertRows();

    /* The row must be the position the child was actually inserted at. The
     * original template hard-coded 0, so every item after the first got an
     * index pointing at the wrong row. */
    return createIndex(newRow, 0, childPart);
}


void ModelPartList::refreshItem( const QModelIndex& index ) {
    if( !index.isValid() )
        return;

    const QModelIndex left  = index.siblingAtColumn(0);
    const QModelIndex right = index.siblingAtColumn(rootItem->columnCount() - 1);

    emit dataChanged(left, right);
}
