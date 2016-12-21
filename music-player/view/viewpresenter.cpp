/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "viewpresenter.h"

#include "../presenter/presenter.h"

ViewPresenter::ViewPresenter(QObject *parent) : QObject(parent)
{

}

void ViewPresenter::binding(Presenter *presenter)
{
    connect(this, &ViewPresenter::play,
            presenter, &Presenter::onMusicPlay);
    connect(this, &ViewPresenter::resume,
            presenter, &Presenter::onMusicResume);
    connect(this, &ViewPresenter::pause,
            presenter, &Presenter::onMusicPause);
    connect(this, &ViewPresenter::prev,
            presenter, &Presenter::onMusicPrev);
    connect(this, &ViewPresenter::next,
            presenter, &Presenter::onMusicNext);
    connect(this, &ViewPresenter::changeProgress,
            presenter, &Presenter::onChangeProgress);
    connect(this, &ViewPresenter::toggleFavourite,
            presenter, &Presenter::onToggleFavourite);
    connect(this, &ViewPresenter::modeChanged,
            presenter, &Presenter::onPlayModeChanged);
    connect(this, &ViewPresenter::locateMusicAtAll,
            presenter, &Presenter::onLocateMusicAtAll);
    connect(this, &ViewPresenter::musicClicked,
            presenter, &Presenter::onMusicPlay);
    connect(this, &ViewPresenter::musicAdd,
            presenter, &Presenter::onMusicAdd);
    connect(this, &ViewPresenter::musicListRemove,
            presenter, &Presenter::onMusicRemove);
    connect(this, &ViewPresenter::musicListDelete,
            presenter, &Presenter::onMusicDelete);

    connect(this, &ViewPresenter::requestCustomContextMenu,
            presenter, &Presenter::onRequestMusiclistMenu);
    connect(this, &ViewPresenter::resort,
            presenter, &Presenter::onResort);

    connect(this, &ViewPresenter::playall,
            presenter, &Presenter::onPlayall);
    connect(this, &ViewPresenter::addPlaylist,
            presenter, &Presenter::onPlaylistAdd);
    connect(this, &ViewPresenter::selectPlaylist,
            presenter, &Presenter::onSelectedPlaylistChanged);


    connect(presenter, &Presenter::metaInfoClean,
            this, &ViewPresenter::showImportView);

    connect(presenter, &Presenter::locateMusic,
               this, &ViewPresenter::locateMusic);
    connect(presenter, &Presenter::coverSearchFinished,
               this, &ViewPresenter::coverSearchFinished);
    connect(presenter, &Presenter::musicPlayed,
               this, &ViewPresenter::musicPlayed);
    connect(presenter, &Presenter::musicPaused,
               this, &ViewPresenter::musicPaused);
    connect(presenter, &Presenter::musicStoped,
               this, &ViewPresenter::musicStoped);
    connect(presenter, &Presenter::musicAdded,
               this, &ViewPresenter::musicAdded);
    connect(presenter, &Presenter::musiclistAdded,
               this, &ViewPresenter::musiclistAdded);
    connect(presenter, &Presenter::musicRemoved,
               this, &ViewPresenter::musicRemoved);
    connect(presenter, &Presenter::progrossChanged,
               this, &ViewPresenter::progrossChanged);
}
