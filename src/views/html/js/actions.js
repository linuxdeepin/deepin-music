jQuery(function($){
    function catchEvent(e) {
        e.preventDefault();
        e.stopPropagation();
    }
    $(document).on('click', '[data-songlist-id]', function(e){
        var $target = $(e.target);
        if ($target.hasClass('btn-play') || $target.hasClass('btn-play-all')) {
            catchEvent(e);
            Dmusic.play($target.data('songlist-id'), 'songlist');
        } else if ($target.hasClass('btn-add-all')) {
            catchEvent(e);
            Dmusic.enqueue($target.data('songlist-id'), 'songlist');
        } else if ($target.hasClass('btn-download')) {
            catchEvent(e);
            Dmusic.download($target.data('songlist-id'), 'songlist');
        }
    });
    $(document).on('click', '[data-album-id]', function(e){
        var $target = $(e.target);
        if ($target.hasClass('btn-play') || $target.hasClass('btn-play-all')) {
            catchEvent(e);
            Dmusic.play($target.data('album-id'), 'album');
        } else if ($target.hasClass('btn-add-all')) {
            catchEvent(e);
            Dmusic.enqueue($target.data('album-id'), 'album');
        } else if ($target.hasClass('btn-download')) {
            catchEvent(e);
            Dmusic.download($target.data('album-id'), 'album');
        }
    });
    $(document).on('click', '[data-hit-name]', function(e){
        var $target = $(e.target);
        if ($target.hasClass('btn-play') || $target.hasClass('btn-play-all')) {
            catchEvent(e);
            Dmusic.play($target.data('hit-name'), 'hit');
        } else if ($target.hasClass('btn-add-all')) {
            catchEvent(e);
            Dmusic.enqueue($target.data('hit-name'), 'hit');
        } else if ($target.hasClass('btn-download')) {
            catchEvent(e);
            Dmusic.download($target.data('hit-name'), 'hit');
        }
    });
    $(document).on('click', '[data-artist-name]', function(e){
        var $target = $(e.target);
        if ($target.hasClass('btn-play') || $target.hasClass('btn-play-all')) {
            catchEvent(e);
            Dmusic.play($target.data('artist-name'), 'artist');
        } else if ($target.hasClass('btn-add-all')) {
            catchEvent(e);
            Dmusic.enqueue($target.data('artist-name'), 'artist');
        } else if ($target.hasClass('btn-download')) {
            catchEvent(e);
            Dmusic.download($target.data('artist-name'), 'artist');
        }
    });
    $(document).on('click', '[data-music-id]', function(e){
        var $target = $(e.target);
        if ($target.hasClass('btn-play') || $target.hasClass('btn-play-all') || $target.hasClass('link-play')) {
            catchEvent(e);
            Dmusic.play($target.data('music-id'), 'music');
        } else if ($target.hasClass('btn-add') || $target.hasClass('btn-add-all') || $target.hasClass('btn-enqueue')) {
            catchEvent(e);
            Dmusic.enqueue($target.data('music-id'), 'music');
        } else if ($target.hasClass('btn-download')) {
            catchEvent(e);
            Dmusic.download($target.data('music-id'), 'music');
        } else if ($target.hasClass('btn-favorite')) {
            catchEvent(e);
            var req;
            if ($target.hasClass('active')) {
                req = Dmusic.removeFavorite($target.data('music-id'));
                if (req.success) {
                    $target.removeClass('active');
                }
            } else {
                req = Dmusic.addFavorite($target.data('music-id'));
                if (req.success) {
                    $target.addClass('active');
                }
            }
        }
    });
    $(document).on('click', '.btn-play-current', function(e){
        e.preventDefault();
        var $target = $(e.target),
            music = $('.playlist li').toArray().map(function(v){
                return $(v).data('music-id');
            });
        Dmusic.play(music, 'music');
    });
    $(document).on('click', '.playlist', function(e){
        var $target = $(e.target),
            $item = $target.parents().filter('li'),
            $cur = $target.parent().addBack().filter('li');
        if($target.parents().filter('.title').length > 0) {
            return Dmusic.play($item.data('music-id'), 'music');
        }
        if($cur.length > 0) {
            if ($cur.hasClass('active')) {
                $cur.removeClass('active');
            } else {
                $('.playlist li').removeClass('active');
                $cur.addClass('active');
            }
            return;
        }
    });
});