#include "musiclistmodel.h"

#include <QDebug>
#include "core/medialibrary.h"

class MusiclistModelPrivate
{
public:
    MusiclistModelPrivate(MusiclistModel *parent) : q_ptr(parent) {}

    PlaylistPtr playlist;

    MusiclistModel *q_ptr;
    Q_DECLARE_PUBLIC(MusiclistModel)
};

MusiclistModel::MusiclistModel(QObject *parent) :
    QStandardItemModel(parent), d_ptr(new MusiclistModelPrivate(this))
{

}

MusiclistModel::MusiclistModel(int rows, int columns, QObject *parent):
    QStandardItemModel(rows, columns, parent), d_ptr(new MusiclistModelPrivate(this))
{

}

MusiclistModel::~MusiclistModel()
{

}

PlaylistPtr MusiclistModel::playlist() const
{
    Q_D(const MusiclistModel);
    return d->playlist;
}

void MusiclistModel::setPlaylist(PlaylistPtr playlist)
{
    Q_D(MusiclistModel);
    d->playlist = playlist;
}

QModelIndex MusiclistModel::findIndex(const MetaPtr meta)
{
    Q_ASSERT(!meta.isNull());
    Q_D(MusiclistModel);

    auto row = d->playlist->index(meta->hash);
    auto itemIndex = this->index(row, 0);

    auto itemHash = data(itemIndex).toString();
    if (itemHash == meta->hash) {
        return itemIndex;
    }

    qCritical() << "search index failed" << meta->hash << itemHash;

    for (int i = 0; i < rowCount(); ++i) {
        itemIndex = index(i, 0);
        auto itemHash = data(itemIndex).toString();
        if (itemHash == meta->hash) {
            return itemIndex;
        }
    }
    return QModelIndex();
}

bool MusiclistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QStandardItemModel::setData(index, value, role);
}

MetaPtr MusiclistModel::meta(const QModelIndex &index, int role) const
{
    auto hash = this->data(index, role).toString();
    return MediaLibrary::instance()->meta(hash);
}

//#include <QMimeData>

//bool MusiclistModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
//{
//    if (action == Qt::IgnoreAction)
//        return true;

//    if (!data->hasFormat("text/uri-list"))
//        return false;

//    if (column > 0)
//        return false;

//    int beginRow;

//    if (row != -1)
//        beginRow = row;
//    else if (parent.isValid())
//        beginRow = parent.row();
//}

Qt::DropActions MusiclistModel::supportedDropActions() const
{
//    return QAbstractItemModel::supportedDropActions();
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags MusiclistModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    } else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}
