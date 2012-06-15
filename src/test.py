
import gtk
from dtk.ui.utils import run_with_profile
from widget.lyrics import LyricsScroll

def main():
    lyrics = LyricsScroll()
    window = lyrics.scroll_window
    window.show_all()
    gtk.main()
    
    
if __name__ == "__main__":
    run_with_profile(main, "/home/evilbeast/a.log")