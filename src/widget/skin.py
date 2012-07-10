from dtk.ui.theme import Theme, ui_theme
from dtk.ui.skin_config import skin_config
import os
from dtk.ui.utils import get_parent_dir

# Init skin config.
skin_config.init_skin(
    "blue",
    os.path.join(get_parent_dir(__file__, 3), "skin"),
    os.path.expanduser("~/.config/deepin-music-player/skin"),
    os.path.expanduser("~/.config/deepin-music-player/skin_config.ini"),
    "dmusic",
    "1.0"
    )

# Create application theme.
app_theme = Theme(
    os.path.join(get_parent_dir(__file__, 3), "app_theme"),
    os.path.expanduser("~/.config/deepin-music-player/theme")
    )

# Set theme.
skin_config.load_themes(ui_theme, app_theme)
skin_config.set_application_window_size(816, 625)

