from dtk.ui.theme import Theme, ui_theme
from dtk.ui.skin_config import skin_config
import os
from dtk.ui.utils import get_parent_dir

skin_config.load_skin(
    "03",
    os.path.join(get_parent_dir(__file__, 3), "skin"),
    os.path.expanduser("~/.config/deepin-music-player/skin")
    )
ui_theme.load_theme()
app_theme = Theme(
    os.path.join(get_parent_dir(__file__, 3), "app_theme"),
    os.path.expanduser("~/.config/deepin-music-player/theme"))
app_theme.load_theme()
