#include "finderwidget.h"

#include "thlibrary/thblackbar.h"

#include "breadcrumbwidget.h"

#include "playlistmodel.h"
#include "playlistview.h"

#include "model/artist.h"
#include "model/album.h"

#include "artistlistview.h"
#include "artistsqlmodel.h"

#include "albumlistview.h"
#include "albumsqlmodel.h"

#include "tracklistview.h"
#include "tracksqlmodel.h"

#include "filesystemfinderview.h"
#include "filesystemmodel.h"
#include "filteringfilesystemmodel.h"

#include "database.h"
#include <QtSql>

static const char* FINDER_VIEW_KEY = "finderView";

FinderWidget::FinderWidget(QWidget *parent) : QWidget(parent) {

    history = new QStack<QWidget*>();

    fileSystemView = 0;
    artistListView = 0;
    albumListView = 0;
    trackListView = 0;

    fileSystemModel = 0;
    artistListModel = 0;
    albumListModel = 0;
    trackListModel = 0;

    // colors
    QPalette p = palette();
    p.setBrush(QPalette::Window, Qt::black);
    p.setBrush(QPalette::Text, Qt::white);
    p.setBrush(QPalette::WindowText, Qt::white);
    p.setBrush(QPalette::ButtonText, Qt::white);
    setPalette(p);
    setAutoFillBackground(true);

    QBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);

    setupBar();
    layout->addWidget(finderBar);

    breadcrumb = new BreadcrumbWidget(this);
    breadcrumb->hide();
    connect(breadcrumb, SIGNAL(goneBack()), SLOT(goBack()));
    layout->addWidget(breadcrumb);

    folderBreadcrumb = new BreadcrumbWidget(this);
    folderBreadcrumb->hide();
    connect(folderBreadcrumb, SIGNAL(goneBack()), SLOT(folderGoBack()));
    layout->addWidget(folderBreadcrumb);

    stackedWidget = new QStackedWidget(this);

    layout->addWidget(stackedWidget);
    setLayout(layout);

    setMinimumWidth(150 * 3 + 8 + style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    setMinimumHeight(150 + finderBar->minimumHeight());

    // Restore saved view
    QSettings settings;
    QString currentViewName = settings.value(FINDER_VIEW_KEY).toString();
    if (currentViewName == "folders") QTimer::singleShot(0, this, SLOT(showFolders()));
    else if (currentViewName == "albums") QTimer::singleShot(0, this, SLOT(showAlbums()));
    else QTimer::singleShot(0, this, SLOT(showArtists()));

}

void FinderWidget::setupBar() {
    finderBar = new THBlackBar(this);

    artistsAction = new QAction(tr("Artists"), this);
    QKeySequence keySequence(Qt::CTRL + Qt::Key_1);
    artistsAction->setShortcut(keySequence);
    artistsAction->setStatusTip(artistsAction->text() + " (" +
                                keySequence.toString(QKeySequence::NativeText) + ")");
    addAction(artistsAction);
    connect(artistsAction, SIGNAL(triggered()), SLOT(showArtists()));
    finderBar->addAction(artistsAction);

    albumsAction = new QAction(tr("Albums"), this);
    keySequence = QKeySequence(Qt::CTRL + Qt::Key_2);
    albumsAction->setShortcut(keySequence);
    albumsAction->setStatusTip(albumsAction->text() + " (" +
                               keySequence.toString(QKeySequence::NativeText) + ")");
    addAction(albumsAction);
    connect(albumsAction, SIGNAL(triggered()), SLOT(showAlbums()));
    finderBar->addAction(albumsAction);

    foldersAction = new QAction(tr("Folders"), this);
    keySequence = QKeySequence(Qt::CTRL + Qt::Key_3);
    foldersAction->setShortcut(keySequence);
    foldersAction->setStatusTip(foldersAction->text() + " (" +
                                keySequence.toString(QKeySequence::NativeText) + ")");
    addAction(foldersAction);
    connect(foldersAction, SIGNAL(triggered()), SLOT(showFolders()));
    finderBar->addAction(foldersAction);

    finderBar->setCheckedAction(0);
}

void FinderWidget::setupArtists() {
    artistListModel = new ArtistSqlModel(this);
    artistListView = new ArtistListView(this);
    artistListView->setEnabled(false);
    connect(artistListView, SIGNAL(activated(const QModelIndex &)), SLOT(artistActivated(const QModelIndex &)));
    connect(artistListView, SIGNAL(play(const QModelIndex &)), SLOT(artistPlayed(const QModelIndex &)));
    connect(artistListView, SIGNAL(entered(const QModelIndex &)), SLOT(artistEntered(const QModelIndex &)));
    connect(artistListView, SIGNAL(viewportEntered()), artistListModel, SLOT(clearHover()));
    artistListView->setModel(artistListModel);
    stackedWidget->addWidget(artistListView);
}

void FinderWidget::setupAlbums() {
    albumListModel = new AlbumSqlModel(this);
    albumListView = new AlbumListView(this);
    albumListView->setEnabled(false);
    connect(albumListView, SIGNAL(activated(const QModelIndex &)), SLOT(albumActivated(const QModelIndex &)));
    connect(albumListView, SIGNAL(play(const QModelIndex &)), SLOT(albumPlayed(const QModelIndex &)));
    connect(albumListView, SIGNAL(entered(const QModelIndex &)), SLOT(albumEntered(const QModelIndex &)));
    connect(albumListView, SIGNAL(viewportEntered()), albumListModel, SLOT(clearHover()));
    albumListView->setModel(albumListModel);
    stackedWidget->addWidget(albumListView);
}

void FinderWidget::setupTracks() {
    trackListModel = new TrackSqlModel(this);
    trackListView = new TrackListView(this);
    connect(trackListView, SIGNAL(activated(const QModelIndex &)), SLOT(trackActivated(const QModelIndex &)));
    connect(trackListView, SIGNAL(play(const QModelIndex &)), SLOT(trackPlayed(const QModelIndex &)));
    connect(trackListView, SIGNAL(entered(const QModelIndex &)), SLOT(trackEntered(const QModelIndex &)));
    connect(trackListView, SIGNAL(viewportEntered()), trackListModel, SLOT(clearHover()));
    trackListView->setModel(trackListModel);
    stackedWidget->addWidget(trackListView);
}

void FinderWidget::setupFolders() {
    fileSystemModel = new FileSystemModel(this);
    fileSystemModel->setResolveSymlinks(true);
    fileSystemModel->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    FilteringFileSystemModel *proxyModel = new FilteringFileSystemModel(this);
    proxyModel->setSourceModel(fileSystemModel);

    fileSystemView = new FileSystemFinderView(this);
    connect(fileSystemView, SIGNAL(activated(const QModelIndex &)), SLOT(folderActivated(const QModelIndex &)));
    connect(fileSystemView, SIGNAL(play(const QModelIndex &)), SLOT(folderPlayed(const QModelIndex &)));
    connect(fileSystemView, SIGNAL(entered(const QModelIndex &)), SLOT(folderEntered(const QModelIndex &)));
    connect(fileSystemView, SIGNAL(viewportEntered()), fileSystemModel, SLOT(clearHover()));
    fileSystemView->setModel(proxyModel);
    fileSystemView->setFileSystemModel(fileSystemModel);
    stackedWidget->addWidget(fileSystemView);
}

void FinderWidget::showArtists() {
    if (!artistListView) setupArtists();

    artistListModel->setQuery("select id from artists where trackCount>1 order by trackCount desc",
                              Database::instance().getConnection());
    if (artistListModel->lastError().isValid())
        qDebug() << artistListModel->lastError();

    showWidget(artistListView, true);
    finderBar->setCheckedAction(artistsAction);
    QSettings settings;
    settings.setValue(FINDER_VIEW_KEY, "artists");
}

void FinderWidget::showAlbums() {
    if (!albumListView) setupAlbums();

    albumListModel->setQuery(
            "select id from albums where trackCount>1 order by artist, year desc, trackCount desc",
            Database::instance().getConnection());
    if (albumListModel->lastError().isValid())
        qDebug() << albumListModel->lastError();

    showWidget(albumListView, true);
    finderBar->setCheckedAction(albumsAction);
    QSettings settings;
    settings.setValue(FINDER_VIEW_KEY, "albums");
}

void FinderWidget::showFolders() {
    if (!fileSystemView) setupFolders();

    const QString path = Database::instance().collectionRoot();
    fileSystemModel->setRootPath(path);
    QSortFilterProxyModel *proxyModel = static_cast<QSortFilterProxyModel*>(fileSystemView->model());
    fileSystemView->setRootIndex(proxyModel->mapFromSource(fileSystemModel->index(path)));

    showWidget(fileSystemView, true);
    finderBar->setCheckedAction(foldersAction);
    QSettings settings;
    settings.setValue(FINDER_VIEW_KEY, "folders");
}

void FinderWidget::showWidget(QWidget *widget, bool isRoot) {
    // breadcrumb behaviour
    if (isRoot) {
        history->clear();
        breadcrumb->clear();
        folderBreadcrumb->clear();
        folderBreadcrumb->hide();
    } else {
        breadcrumb->addItem(widget->windowTitle());
    }
    breadcrumb->setVisible(!isRoot);

    // call disappear() on previous widget
    QWidget* currentWidget = stackedWidget->currentWidget();
    if (currentWidget) {
        bool ret = QMetaObject::invokeMethod(currentWidget, "disappear", Qt::DirectConnection);
        if (!ret) qDebug() << "FinderWidget::showWidget invokeMethod failed for" << currentWidget;
    }

    // call appear() on new widget
    bool ret = QMetaObject::invokeMethod(widget, "appear", Qt::DirectConnection);
    if (!ret) qDebug() << "FinderWidget::showWidget invokeMethod failed for" << widget;

    stackedWidget->setCurrentWidget(widget);
    history->push(widget);
}

void FinderWidget::goBack() {
    if (history->size() > 1) {
        breadcrumb->goBack();
        breadcrumb->goBack();
        history->pop();
        QWidget *widget = history->pop();
        bool isRoot = history->isEmpty();
        showWidget(widget, isRoot);
    }
}

void FinderWidget::folderGoBack() {
    folderBreadcrumb->goBack();
    QModelIndex index = fileSystemView->rootIndex();
    QSortFilterProxyModel *proxyModel = static_cast<QSortFilterProxyModel*>(fileSystemView->model());
    if (proxyModel) {
        index = proxyModel->mapToSource(index);
        QString path = fileSystemModel->filePath(index);
        QDir dir(path);
        dir.cdUp();
        // qDebug() << d << folderListModel->rootPath();
        if (dir.absolutePath() == fileSystemModel->rootDirectory().absolutePath()) {
            folderBreadcrumb->clear();
            folderBreadcrumb->hide();
        }
        index = fileSystemModel->index(dir.absolutePath(), 0);
        qDebug() << dir.absolutePath() << index.isValid();
        index = proxyModel->mapFromSource(index);
        fileSystemView->setRootIndex(index);
    }
}

void FinderWidget::appear() {
    QWidget *currentWidget = stackedWidget->currentWidget();
    if (currentWidget) {
        bool success = QMetaObject::invokeMethod(stackedWidget->currentWidget(), "appear", Qt::DirectConnection);
        if (!success) qDebug() << "Error invoking appear() on" << stackedWidget->currentWidget();
    }
}

void FinderWidget::artistEntered ( const QModelIndex & index ) {
    artistListModel->setHoveredRow(index.row());
}

void FinderWidget::artistActivated ( const QModelIndex & index ) {
    if (!albumListView) setupAlbums();

    // get the data object
    const ArtistPointer artistPointer = index.data(Finder::DataObjectRole).value<ArtistPointer>();
    Artist *artist = artistPointer.data();

    QString qry("select id from albums where artist=%1 and trackCount>0 order by year desc, trackCount desc");
    qry = qry.arg(artist->getId());
    qDebug() << qry;
    albumListModel->setQuery(qry, Database::instance().getConnection());
    if (albumListModel->lastError().isValid())
        qDebug() << albumListModel->lastError();

    albumListView->setWindowTitle(artist->getName());

    showWidget(albumListView, false);
}

void FinderWidget::artistPlayed ( const QModelIndex & index ) {
    const ArtistPointer artistPointer = index.data(Finder::DataObjectRole).value<ArtistPointer>();
    Artist *artist = artistPointer.data();
    QList<Track*> tracks = artist->getTracks();
    addTracksAndPlay(tracks);
}

void FinderWidget::albumEntered ( const QModelIndex & index ) {
    albumListModel->setHoveredRow(index.row());
}

void FinderWidget::albumActivated ( const QModelIndex & index ) {
    if (!trackListView) setupTracks();

    const AlbumPointer albumPointer = index.data(Finder::DataObjectRole).value<AlbumPointer>();
    Album *album = albumPointer.data();

    QString qry("select id from tracks where album=%1 order by track, path");
    qry = qry.arg(album->getId());
    qDebug() << qry;
    trackListModel->setQuery(qry, Database::instance().getConnection());
    if (trackListModel->lastError().isValid())
        qDebug() << trackListModel->lastError();

    trackListView->setWindowTitle(album->getName());
    showWidget(trackListView, false);

}

void FinderWidget::albumPlayed ( const QModelIndex & index ) {
    const AlbumPointer albumPointer = index.data(Finder::DataObjectRole).value<AlbumPointer>();
    Album *album = albumPointer.data();
    QList<Track*> tracks = album->getTracks();
    addTracksAndPlay(tracks);
}

void FinderWidget::trackEntered ( const QModelIndex & index ) {
    trackListModel->setHoveredRow(index.row());
}

void FinderWidget::trackActivated ( const QModelIndex & index ) {
    const TrackPointer trackPointer = index.data(Finder::DataObjectRole).value<TrackPointer>();
    Track *track = trackPointer.data();
    playlistModel->addTrack(track);
    playlistModel->setActiveRow(playlistModel->rowForTrack(track));
}

void FinderWidget::folderEntered ( const QModelIndex & index ) {
    fileSystemModel->setHoveredRow(index.row());
}

void FinderWidget::folderActivated(const QModelIndex & index) {
    const TrackPointer trackPointer = index.data(Finder::DataObjectRole).value<TrackPointer>();
    Track *track = trackPointer.data();
    if (track) {
        addTracksAndPlay(track->getTracks());
    } else {
        fileSystemView->setRootIndex(index);
        const FolderPointer folderPointer = index.data(Finder::DataObjectRole).value<FolderPointer>();
        Folder *folder = folderPointer.data();
        if (folder) {
            folderBreadcrumb->addItem(folder->getName());
            folderBreadcrumb->setVisible(true);
        }
    }
}

void FinderWidget::folderPlayed(const QModelIndex & index) {
    const FolderPointer folderPointer = index.data(Finder::DataObjectRole).value<FolderPointer>();
    Folder *folder = folderPointer.data();
    if (folder) {
        QList<Track*> tracks = folder->getTracks();
        addTracksAndPlay(tracks);
    } else {
        const TrackPointer trackPointer = index.data(Finder::DataObjectRole).value<TrackPointer>();
        Track *track = trackPointer.data();
        if (track) {
            addTracksAndPlay(track->getTracks());
        }
    }
}

void FinderWidget::addTracksAndPlay(QList<Track *>tracks) {
    if (tracks.isEmpty()) return;
    playlistModel->addTracks(tracks);

    Track* trackToPlay = 0;

    QSettings settings;
    const bool shuffle = settings.value("shuffle").toBool();
    if (shuffle) {
        int nextRow = (int) ((float) qrand() / (float) RAND_MAX * tracks.size());
        trackToPlay = tracks.at(nextRow);
    } else {
        trackToPlay = tracks.first();
    }
    playlistModel->setActiveRow(playlistModel->rowForTrack(trackToPlay));
    playlistView->scrollTo(playlistModel->indexForTrack(trackToPlay), QAbstractItemView::PositionAtCenter);
}
