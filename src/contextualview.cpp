#include "contextualview.h"
#include "context/artistinfo.h"
#include "context/albuminfo.h"
#include "context/trackinfo.h"
#include "model/track.h"

ContextualView::ContextualView(QWidget *parent) :
        QWidget(parent) {


    setAutoFillBackground(true);

    // colors
    QPalette p = palette();
    p.setBrush(QPalette::Window, Qt::black);
    p.setBrush(QPalette::Foreground, Qt::white);
    p.setBrush(QPalette::Base, Qt::black);
    p.setBrush(QPalette::Text, Qt::white);
    p.setColor(QPalette::Link, Qt::white);
    p.setBrush(QPalette::LinkVisited, Qt::white);
    this->setPalette(p);

    QBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    artistInfo = new ArtistInfo(this);
    artistInfo->setPalette(p);
    layout->addWidget(artistInfo);

    albumInfo = new AlbumInfo(this);
    albumInfo->setPalette(p);
    layout->addWidget(albumInfo);

    trackInfo = new TrackInfo(this);
    trackInfo->setPalette(p);
    layout->addWidget(trackInfo);

    setLayout(layout);
    
}

void ContextualView::setTrack(Track *track) {
    artistInfo->setArtist(track->getArtist());
    albumInfo->setAlbum(track->getAlbum());
    trackInfo->setTrack(track);
}

void ContextualView::disappear() {
    albumInfo->clear();

    // this fixes the vertical scrollbar
    albumInfo->adjustSize();
    adjustSize();
}

void ContextualView::paintEvent(QPaintEvent *event) {
    /*
    QPainter painter(this);
    QLinearGradient linearGrad(0, 0, 0, height());
    linearGrad.setColorAt(0, QColor(0x34, 0x34, 0x34));
    linearGrad.setColorAt(1, QColor(0x00, 0x00, 0x00));
    painter.fillRect(0, 0, width(), height(), QBrush(linearGrad));
    */
}
