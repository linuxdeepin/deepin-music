from dtk.ui.theme import Theme, ui_theme
from dtk.ui.skin_config import skin_config
import os
from deepin_utils.file import get_parent_dir
from constant import FULL_DEFAULT_WIDTH, FULL_DEFAULT_HEIGHT, PROGRAM_VERSION

# Init skin config.
skin_config.init_skin(
    "sky",
    os.path.join(get_parent_dir(__file__, 3), "skin"),
    os.path.expanduser("~/.config/deepin-music-player/skin"),
    os.path.expanduser("~/.config/deepin-music-player/skin_config.ini"),
    "dmusic",
    PROGRAM_VERSION
    )

# Create application theme.
app_theme = Theme(
    os.path.join(get_parent_dir(__file__, 3), "app_theme"),
    os.path.expanduser("~/.config/deepin-music-player/theme")
    )

# Set theme.
skin_config.load_themes(ui_theme, app_theme)
skin_config.set_application_window_size(FULL_DEFAULT_WIDTH, FULL_DEFAULT_HEIGHT)

