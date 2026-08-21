# Context Bundle — deepin-music

## 1. 元素-功能对照表

| 元素名 | Role | 功能描述 | 可见条件 |
|--------|------|----------|----------|
| Add Songs | push button | 添加音乐文件（点击打开文件对话框选择音乐） | 专辑默认页（无音乐时） |
| Open Folders | push button | 添加音乐文件夹（点击打开文件夹选择对话框） | 专辑默认页（无音乐时） |
| RoundedImage_CircularButton | push button | 专辑封面圆形播放按钮（点击播放该专辑） | 专辑网格视图 |
| Morebutton | push button | 专辑项更多操作按钮 | 专辑网格视图 |
| ItemDelegate | list item | 专辑列表项代理 | 专辑网格视图 |
| Imagecell | list item | 专辑列表项图片单元格 | 专辑列表视图 |
| ListView_AlbumListDelegate | list item | 专辑列表项代理 | 专辑列表视图 |
| Albums | push button | 专辑视图标题按钮（切换到专辑视图） | 专辑视图 |
| CircularButton_Button | push button | 专辑/歌曲项圆形播放按钮 | 列表/网格项悬停时 |
| SortBtn | push button | 排序按钮（打开排序菜单） | 点击排序按钮后弹出 |
| Menu | menu | 排序菜单 | 点击排序按钮后弹出 |
| title | menu item | 排序菜单标题项 | 点击排序按钮后弹出 |
| Play All | push button | 播放全部按钮（播放当前列表所有歌曲） | 子列表标题区域 |
| DataSort | menu | 数据排序菜单 | 工具栏区域 |
| GridViewButton | push button | 网格视图切换按钮 | 工具栏区域 |
| ListViewButton | push button | 列表视图切换按钮 | 工具栏区域 |
| Cancel | push button | 取消按钮（关闭对话框） | CD移除对话框弹出时 |
| Minimize to system tray | radio button | 最小化到系统托盘单选项 | 关闭确认对话框弹出时 |
| Exit | radio button | 退出单选项 | 关闭确认对话框弹出时 |
| Do not ask again | check box | 不再询问复选框 | 关闭确认对话框弹出时 |
| Cancel | push button | 取消按钮（关闭对话框） | 关闭确认对话框弹出时 |
| Confirm | push button | 确认按钮（确认对话框操作） | 关闭确认对话框弹出时 |
| Cancel | push button | 取消按钮（关闭对话框） | 删除本地文件确认对话框 |
| Cancel | push button | 取消按钮（关闭对话框） | 删除歌单确认对话框 |
| SwitchBtn | toggle button | 均衡器开关切换按钮 | 均衡器对话框弹出时 |
| SelectComBox | combo box | 均衡器模式选择下拉框 | 均衡器对话框弹出时 |
| Save | push button | 保存按钮（保存均衡器设置） | 均衡器对话框弹出时 |
| Reset | push button | 重置按钮（重置均衡器设置） | 均衡器对话框弹出时 |
| PreamplifierSlider | slider | 前置放大增益滑块 | 均衡器对话框弹出时 |
| DelegateSlider | slider | 频率波段滑块 | 均衡器对话框弹出时 |
| OK | push button | 确定按钮（关闭导入失败提示） | 导入失败提示对话框 |
| Title | list item | 歌曲信息-标题字段 | 歌曲信息对话框弹出时 |
| Artist | list item | 歌曲信息-艺术家字段 | 歌曲信息对话框弹出时 |
| Album | list item | 歌曲信息-专辑字段 | 歌曲信息对话框弹出时 |
| Type | list item | 歌曲信息-文件类型字段 | 歌曲信息对话框弹出时 |
| Size | list item | 歌曲信息-文件大小字段 | 歌曲信息对话框弹出时 |
| Duration | list item | 歌曲信息-时长字段 | 歌曲信息对话框弹出时 |
| Path | list item | 歌曲信息-文件路径字段 | 歌曲信息对话框弹出时 |
| title | push button | 排序菜单标题项 | 属性项区域 |
| SettingsDialog_CheckBox | check box | 设置-播放选项复选框1（启动时自动播放） | 设置对话框弹出时 |
| SettingsDialog_CheckBox_2 | check box | 设置-播放选项复选框2（记住上次播放进度） | 设置对话框弹出时 |
| SettingsDialog_CheckBox_3 | check box | 设置-播放选项复选框3（开启淡入淡出） | 设置对话框弹出时 |
| Minimize to system tray | radio button | 最小化到系统托盘单选项 | 设置对话框弹出时 |
| Exit | radio button | 退出单选项 | 设置对话框弹出时 |
| Ask me always | radio button | 每次询问单选项（关闭主窗口时弹出确认） | 设置对话框弹出时 |
| Cancel | push button | 取消按钮（关闭对话框） | 设置对话框弹出时 |
| Replace | push button | 替换按钮（快捷键替换确认） | 设置对话框弹出时 |
| Restore Defaults | push button | 恢复默认按钮（恢复快捷键默认设置） | 设置对话框弹出时 |
| Component_Menu | menu | 系统托盘右键菜单 | 主窗口 |
| Play/Pause | menu item | 托盘菜单-播放/暂停 | 主窗口 |
| Previous | menu item | 托盘菜单-上一首 | 主窗口 |
| Next | menu item | 托盘菜单-下一首 | 主窗口 |
| Exit | menu item | 退出单选项 | 主窗口 |
| TabArea | page tab list | 搜索结果标签页区域 | 搜索结果窗口 |
| Music | page tab | 搜索结果-音乐标签页 | 搜索结果窗口 |
| Album | page tab | 歌曲信息-专辑字段 | 搜索结果窗口 |
| Artist | page tab | 歌曲信息-艺术家字段 | 搜索结果窗口 |
| AllMusicSortMenu | menu | 所有音乐排序菜单 | 搜索结果窗口 |
| AlbumMusicSortMenu | menu | 专辑音乐排序菜单 | 搜索结果窗口 |
| ArtistMusicSortMenu | menu | 演唱者音乐排序菜单 | 搜索结果窗口 |
| MusicSingerGridItem | list item | 搜索结果-歌手网格项 | 搜索结果窗口 |
| ItemDelegate_2 | list item | 搜索结果-列表项代理 | 搜索结果窗口 |
| LrcBtn | push button | 歌词按钮（切换歌词界面） | 主窗口工具栏 |
| VolumeBtn | push button | 音量按钮（打开音量调节） | 主窗口工具栏 |
| ListBtn | push button | 播放列表按钮（打开播放队列） | 主窗口工具栏 |
| WindowTitlebar_Menu | menu | 标题栏主菜单按钮 | 标题栏 |
| Add playlist | menu item | 主菜单-添加播放列表 | 标题栏 |
| Add music | menu item | 主菜单-添加音乐 | 标题栏 |
| Settings | menu item | 主菜单-设置 | 标题栏 |
| Add Songs | push button | 添加音乐文件（点击打开文件对话框选择音乐） | 所有音乐默认页（无音乐时） |
| Open Folders | push button | 添加音乐文件夹（点击打开文件夹选择对话框） | 所有音乐默认页（无音乐时） |
| listTitle | push button | 所有音乐列表标题按钮 | 所有音乐列表 |
| Imagecell_4 | list item | 所有音乐列表项图片 | 所有音乐列表项 |
| ItemD | list item | 所有音乐列表项 | 所有音乐列表视图 |
| SidebarItem | list item | 侧边栏列表项 | 侧边栏 |
| Item | push button | 侧边栏项代理按钮 | 侧边栏项代理 |
| MoreMenu | menu | 专辑右键更多菜单 | 右键点击专辑时 |
| View details | menu item | 右键菜单-查看详情 | 右键点击专辑时 |
| Play all | menu item | 右键菜单-播放全部 | 右键点击专辑时 |
| Add to | menu | 右键菜单-添加到（子菜单） | 右键点击专辑时 |
| MoreMenu_3 | menu | 演唱者右键更多菜单 | 右键点击演唱者时 |
| View details | menu item | 右键菜单-查看详情 | 右键点击演唱者时 |
| Play all | menu item | 右键菜单-播放全部 | 右键点击演唱者时 |
| Add to | menu | 右键菜单-添加到（子菜单） | 右键点击演唱者时 |
| ImportMenu | menu | 导入菜单（添加到播放队列/我的收藏/新建歌单） | 右键菜单-添加到子菜单 |
| Play queue | menu item | 导入菜单-播放队列 | 右键菜单-添加到子菜单 |
| My favorites | menu item | 导入菜单-我的收藏 | 右键菜单-添加到子菜单 |
| Create new playlist | menu item | 导入菜单-新建歌单 | 右键菜单-添加到子菜单 |
| &lt; | menu item | &lt; (menu item) | 右键菜单-添加到子菜单 |
| MulitSelectMenu | menu | 多选右键菜单 | 多选时右键菜单 |
| Add to | menu | 右键菜单-添加到（子菜单） | 多选时右键菜单 |
| play | menu item | 右键菜单-播放 | 多选时右键菜单 |
| Delete from local disk | menu item | 右键菜单-从本地磁盘删除 | 多选时右键菜单 |
| MoreMenu_2 | menu | 音乐右键更多菜单 | 右键点击音乐时 |
| Pause | menu item | 右键菜单-暂停 | 右键点击音乐时 |
| Add to | menu | 右键菜单-添加到（子菜单） | 右键点击音乐时 |
| Open in file manager | menu item | 右键菜单-在文件管理器中打开 | 右键点击音乐时 |
| play | menu item | 右键菜单-播放 | 右键点击音乐时 |
| Delete from local disk | menu item | 右键菜单-从本地磁盘删除 | 右键点击音乐时 |
| Encoding | menu | 右键菜单-编码方式（子菜单） | 右键点击音乐时 |
| encodings | menu item | 编码方式-选择编码 | 右键点击音乐时 |
| Song info | menu item | 右键菜单-歌曲信息 | 右键点击音乐时 |
| PlaylistMenu | menu | 歌单右键菜单 | 右键点击歌单时 |
| Play | menu item | Play (menu item) | 右键点击歌单时 |
| Add songs | menu item | 歌单菜单-添加歌曲 | 右键点击歌单时 |
| Rename | menu item | 歌单菜单-重命名 | 右键点击歌单时 |
| Delete | menu item | 歌单菜单-删除 | 右键点击歌单时 |
| ListView_AlbumSublistDelegate_2 | list item | 专辑子列表项代理 | 专辑子列表 |
| ListView_AlbumSublistDelegate | list item | 专辑子列表项代理 | 专辑子列表视图 |
| SublistDelegate | push button | 演唱者子列表代理按钮 | 演唱者子列表项 |
| Imagecell_2 | list item | 演唱者子列表图片 | 演唱者子列表项 |
| ListView_ArtistSublistDelegate | list item | 演唱者子列表项代理 | 演唱者子列表视图 |
| TitleButton1 | push button | 音乐子列表标题按钮 | 音乐子列表标题 |
| Empty | push button | 播放队列空状态删除按钮 | 播放队列 |
| PlaylistDelegate | list item | 播放队列列表项代理 | 播放队列 |
| Imagecell_3 | list item | 播放队列列表项图片 | 播放队列项 |
| Add Songs | push button | 添加音乐文件（点击打开文件对话框选择音乐） | 演唱者默认页（无音乐时） |
| Open Folders | push button | 添加音乐文件夹（点击打开文件夹选择对话框） | 演唱者默认页（无音乐时） |
| CircularImg_CircularButton | push button | 演唱者封面圆形播放按钮 | 演唱者网格项 |
| Morebutton_2 | push button | 演唱者项更多操作按钮 | 演唱者网格项 |
| Artists | push button | 演唱者视图标题按钮（切换到演唱者视图） | 演唱者视图 |
| MusicSingerGridItem_2 | list item | 演唱者网格项 | 演唱者视图 |
| ContentSlider | slider | 音量调节滑块 | 点击音量按钮后 |

## 2. 界面-元素映射

| 界面 | 包含元素 |
|------|----------|
| 主窗口-标题栏 | WindowTitlebar_Menu, Add playlist, Add music, Settings |
| 主窗口-工具栏 | LrcBtn, VolumeBtn, ListBtn, ContentSlider |
| 主窗口-标签页 | TabArea, Music, Album, Artist |
| 主窗口-系统托盘菜单 | Component_Menu, Play/Pause, Previous, Next, Exit |
| 所有音乐-默认页 | Add Songs, Open Folders |
| 所有音乐-列表 | listTitle, ItemD, Imagecell_4, SortBtn, Play All, DataSort, GridViewButton, ListViewButton |
| 所有音乐-排序菜单 | AllMusicSortMenu, Menu, title |
| 专辑-默认页 | Add Songs, Open Folders |
| 专辑-网格视图 | RoundedImage_CircularButton, Morebutton, CircularButton_Button |
| 专辑-列表视图 | ItemDelegate, Imagecell, ListView_AlbumListDelegate |
| 专辑-视图标题 | Albums |
| 专辑-子列表 | ListView_AlbumSublistDelegate, ListView_AlbumSublistDelegate_2 |
| 专辑-排序菜单 | AlbumMusicSortMenu |
| 演唱者-默认页 | Add Songs, Open Folders |
| 演唱者-网格视图 | CircularImg_CircularButton, Morebutton_2, MusicSingerGridItem_2 |
| 演唱者-视图标题 | Artists |
| 演唱者-子列表 | SublistDelegate, Imagecell_2, ListView_ArtistSublistDelegate |
| 演唱者-排序菜单 | ArtistMusicSortMenu |
| 音乐右键菜单 | MoreMenu_2, Pause, Add to, Open in file manager, play, Delete from local disk, Encoding, encodings, Song info |
| 专辑右键菜单 | MoreMenu, View details, Play all, Add to |
| 演唱者右键菜单 | MoreMenu_3, View details, Play all, Add to |
| 多选右键菜单 | MulitSelectMenu, Add to, play, Delete from local disk |
| 导入子菜单 | ImportMenu, Play queue, My favorites, Create new playlist, < |
| 歌单右键菜单 | PlaylistMenu, Play, Add songs, Rename, Delete |
| 关闭确认对话框 | Cancel, Minimize to system tray, Exit, Do not ask again, Confirm |
| 设置对话框 | SettingsDialog_CheckBox, SettingsDialog_CheckBox_2, SettingsDialog_CheckBox_3, Minimize to system tray, Exit, Ask me always, Cancel, Replace, Restore Defaults |
| 均衡器对话框 | SwitchBtn, SelectComBox, Save, Reset, PreamplifierSlider, DelegateSlider |
| 歌曲信息对话框 | Title, Artist, Album, Type, Size, Duration, Path |
| 导入失败对话框 | OK |
| 播放队列 | Empty, PlaylistDelegate, Imagecell_3 |
| 侧边栏 | SidebarItem, Item |
| 搜索结果窗口 | TabArea, Music, Album, Artist, AllMusicSortMenu, AlbumMusicSortMenu, ArtistMusicSortMenu, MusicSingerGridItem, ItemDelegate_2 |
| 音乐子列表标题 | TitleButton1 |
| 属性项 | title |

## 3. 功能-操作-断言映射

| 功能 | 操作 | 断言目标 |
|------|------|----------|
| 添加音乐文件 | 主菜单 → Add music → 文件对话框选择文件 | ItemD |
| 添加音乐文件夹 | 主菜单 → Add music → 文件对话框选择文件夹 | ItemD |
| 添加音乐(默认页按钮) | element_action Add Songs → file_dialog_select | ItemD |
| 添加音乐文件夹(默认页按钮) | element_action Open Folders → file_dialog_select | ItemD |
| 新建歌单 | 主菜单 → Add playlist | PlaylistDelegate |
| 播放全部 | element_action Play All | PlaylistDelegate |
| 播放/暂停 | element_action Play All (toggle) | PlaylistDelegate |
| 上一首 | keyboard_hot_key (播放控制) | PlaylistDelegate |
| 下一首 | keyboard_hot_key (播放控制) | PlaylistDelegate |
| 打开歌词界面 | element_action LrcBtn | LrcBtn |
| 关闭歌词界面 | element_action LrcBtn (再次点击) | LrcBtn |
| 打开音量调节 | element_action VolumeBtn | ContentSlider |
| 调节音量 | element_action VolumeBtn → element_set_value ContentSlider | ContentSlider |
| 打开播放队列 | element_action ListBtn | PlaylistDelegate |
| 切换到所有音乐 | element_action TabArea → Music | listTitle |
| 切换到专辑视图 | element_action TabArea → Album | Albums |
| 切换到演唱者视图 | element_action TabArea → Artist | Artists |
| 专辑网格视图 | element_action GridViewButton | RoundedImage_CircularButton |
| 专辑列表视图 | element_action ListViewButton | ItemDelegate |
| 排序音乐 | element_action SortBtn → Menu → title | ItemD |
| 搜索音乐 | keyboard_type 关键字 → keyboard_press Enter | ItemDelegate_2 |
| 收藏音乐 | element_action (收藏按钮) | PlaylistDelegate |
| 右键播放音乐 | mouse_right_click 音乐项 → dtk_context_menu play | PlaylistDelegate |
| 右键暂停音乐 | mouse_right_click 音乐项 → dtk_context_menu Pause | PlaylistDelegate |
| 右键删除本地音乐 | mouse_right_click → dtk_context_menu Delete from local disk | ItemD |
| 右键在文件管理器打开 | mouse_right_click → dtk_context_menu Open in file manager | ItemD |
| 右键查看歌曲信息 | mouse_right_click → dtk_context_menu Song info | Title |
| 右键切换编码 | mouse_right_click → dtk_context_menu Encoding → encodings | ItemD |
| 右键添加到播放队列 | mouse_right_click → dtk_context_menu Add to → Play queue | PlaylistDelegate |
| 右键添加到我的收藏 | mouse_right_click → dtk_context_menu Add to → My favorites | PlaylistDelegate |
| 右键添加到新建歌单 | mouse_right_click → dtk_context_menu Add to → Create new playlist | PlaylistDelegate |
| 专辑右键查看详情 | mouse_right_click 专辑 → dtk_context_menu View details | Imagecell |
| 专辑右键播放全部 | mouse_right_click 专辑 → dtk_context_menu Play all | PlaylistDelegate |
| 演唱者右键查看详情 | mouse_right_click 演唱者 → dtk_context_menu View details | MusicSingerGridItem_2 |
| 演唱者右键播放全部 | mouse_right_click 演唱者 → dtk_context_menu Play all | PlaylistDelegate |
| 歌单右键播放 | mouse_right_click 歌单 → dtk_context_menu Play | PlaylistDelegate |
| 歌单右键添加歌曲 | mouse_right_click 歌单 → dtk_context_menu Add songs | PlaylistDelegate |
| 歌单右键重命名 | mouse_right_click 歌单 → dtk_context_menu Rename | SidebarItem |
| 歌单右键删除 | mouse_right_click 歌单 → dtk_context_menu Delete | SidebarItem |
| 多选右键播放 | mouse_right_click 多选 → dtk_context_menu play | PlaylistDelegate |
| 多选右键删除 | mouse_right_click 多选 → dtk_context_menu Delete from local disk | ItemD |
| 打开均衡器 | 主菜单 → 均衡器 | SwitchBtn |
| 开启均衡器 | element_action SwitchBtn | SelectComBox |
| 选择均衡器模式 | element_action SelectComBox | SelectComBox |
| 调节前置放大 | element_set_value PreamplifierSlider | PreamplifierSlider |
| 调节频率波段 | element_set_value DelegateSlider | DelegateSlider |
| 保存均衡器 | element_action Save | SwitchBtn |
| 重置均衡器 | element_action Reset | PreamplifierSlider |
| 打开设置 | 主菜单 → Settings | SettingsDialog_CheckBox |
| 设置自动播放 | element_action SettingsDialog_CheckBox | SettingsDialog_CheckBox |
| 设置记住进度 | element_action SettingsDialog_CheckBox_2 | SettingsDialog_CheckBox_2 |
| 设置淡入淡出 | element_action SettingsDialog_CheckBox_3 | SettingsDialog_CheckBox_3 |
| 设置关闭行为-最小化 | element_action Minimize to system tray | Minimize to system tray |
| 设置关闭行为-退出 | element_action Exit | Exit |
| 设置关闭行为-每次询问 | element_action Ask me always | Ask me always |
| 恢复默认快捷键 | element_action Restore Defaults | Restore Defaults |
| 关闭确认-最小化到托盘 | element_action Minimize to system tray | Minimize to system tray |
| 关闭确认-退出 | element_action Exit | Exit |
| 关闭确认-不再询问 | element_action Do not ask again | Do not ask again |
| 关闭确认-取消 | element_action Cancel | CloseConfirmDialog |
| 关闭确认-确认 | element_action Confirm | CloseConfirmDialog |
| 查看歌曲信息 | 右键 → Song info (断言信息字段) | Title |
| 导入失败确认 | element_action OK | ImportFailedDialog |
| 清空播放队列 | element_action Empty | Empty |
| 切换主题 | 主菜单 → 主题 → 浅色/深色/系统 | WindowTitlebar_Menu |
| 查看帮助 | 主菜单 → 帮助 | WindowTitlebar_Menu |
| 查看关于 | 主菜单 → 关于 | WindowTitlebar_Menu |
| 退出应用(主菜单) | 主菜单 → 退出 | WindowTitlebar_Menu |
| 退出应用(托盘) | element_action Component_Menu → Exit | Component_Menu |
| 快捷键打开快捷键预览 | keyboard_hot_key ctrl+shift+? | WindowTitlebar_Menu |
| 快捷键打开文件 | keyboard_hot_key ctrl+o → file_dialog_select | ItemD |
| Tab键切换焦点 | keyboard_press Tab | LrcBtn |
| Esc关闭对话框 | keyboard_press Escape | SettingsDialog_CheckBox |
| 方向键导航列表 | keyboard_press Down/Up | ItemD |
