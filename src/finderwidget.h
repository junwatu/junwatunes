#ifndef FINDERWIDGET_H
#define FINDERWIDGET_H

#include <QtGui>

class BreadcrumbWidget;
class PlaylistModel;
class PlaylistView;
class THBlackBar;
class ArtistSqlModel;
class AlbumSqlModel;
class TrackSqlModel;
class FileSystemModel;
class FileSystemFinderView;
class Track;

namespace Finder {

    enum FinderDataRoles {
        ItemTypeRole = Qt::UserRole,
        DataObjectRole,
        ActiveItemRole,
        HoveredItemRole,
        PlayIconAnimationItemRole,
        PlayIconHoveredRole
    };

    enum FinderItemTypes {
        ItemTypeGenre = 1,
        ItemTypeArtist ,
        ItemTypeAlbum,
        ItemTypeTrack,
        ItemTypeFolder
    };

}

class FinderWidget : public QWidget {

    Q_OBJECT;

public:
    FinderWidget(QWidget *parent);
    void setPlaylistModel(PlaylistModel *playlistModel) {
        this->playlistModel = playlistModel;
    }
    void setPlaylistView(PlaylistView *playlistView) {
        this->playlistView = playlistView;
    }
    void appear();

private slots:
    void goBack();
    void folderGoBack();
    void showArtists();
    void showAlbums();
    void showFolders();

    void artistEntered(const QModelIndex &index);
    void artistActivated(const QModelIndex &index);
    void artistPlayed(const QModelIndex &index);

    void albumEntered(const QModelIndex &index);
    void albumActivated(const QModelIndex &index);
    void albumPlayed(const QModelIndex &index);

    void trackEntered(const QModelIndex &index);
    void trackActivated(const QModelIndex &index);

    void folderEntered(const QModelIndex &index);
    void folderActivated(const QModelIndex &index);
    void folderPlayed(const QModelIndex &index);

private:
    void setupBar();
    void setupArtists();
    void setupAlbums();
    void setupFolders();
    void setupTracks();
    void showWidget(QWidget *widget, bool isRoot);
    void addTracksAndPlay(QList<Track*> tracks);

    THBlackBar *finderBar;
    QAction *artistsAction;
    QAction *albumsAction;
    QAction *foldersAction;

    QStack<QWidget*> *history;
    QStackedWidget *stackedWidget;
    BreadcrumbWidget *breadcrumb;
    BreadcrumbWidget *folderBreadcrumb;

    PlaylistModel *playlistModel;
    PlaylistView *playlistView;

    FileSystemFinderView *fileSystemView;
    FileSystemModel *fileSystemModel;

    QListView *artistListView;
    ArtistSqlModel *artistListModel;

    QListView *albumListView;
    AlbumSqlModel *albumListModel;

    QListView *trackListView;
    TrackSqlModel *trackListModel;

};

#endif // FINDERWIDGET_H
