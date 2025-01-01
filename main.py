"""
    Class Widgets Csv Import
    让我们用.csv表格编辑 Class Widgets 课表！将.csv文件转换为 Class Widgets Json 课表
    Detrital
    Github Repositories https://github.com/Detritalw/Class-Widget-CSV-import
"""
from PyQt5 import uic
from loguru import logger
from datetime import datetime
from .ClassWidgets.base import PluginBase, SettingsBase, PluginConfig  # 导入CW的基类

from PyQt5.QtWidgets import QHBoxLayout, QPushButton, QFileDialog, QLineEdit
from qfluentwidgets import ImageLabel
import subprocess

class Plugin(PluginBase):  # 插件类
    def __init__(self, cw_contexts, method):  # 初始化
        super().__init__(cw_contexts, method)  # 调用父类初始化方法

        self.cfg = PluginConfig(self.PATH, 'config.json')  # 实例化配置类

    def execute(self):  # 自启动执行部分
        logger.success('Plugin1 executed!')
        logger.info(f'Config path: {self.PATH}')

    def update(self, cw_contexts):  # 自动更新部分
        super().update(cw_contexts)  # 调用父类更新方法
        self.cfg.update_config()  # 更新配置

        if self.method.is_get_notification():
            logger.warning(f'Plugin1 got notification! Title: {self.cw_contexts["Notification"]["title"]}')

            if self.cw_contexts['Notification']['state'] == 0:  # 如果下课
                self.method.subprocess_exec(self.cfg['name'], self.cfg['action'])  # 调用CW方法构建自动化


# 设置页
class Settings(SettingsBase):
    def __init__(self, plugin_path, parent=None):
        super().__init__(plugin_path, parent)
        self.PATH = plugin_path  # 确保 plugin_path 被正确设置
        uic.loadUi(f'{self.PATH}/settings.ui', self)  # 加载设置界面

        # 添加打开转换器按钮
        self.openButton = self.findChild(QPushButton, 'openButton')
        if self.openButton:
            self.openButton.clicked.connect(self.open_converter)
        else:
            logger.error('openButton not found in settings.ui')

        self.helpButton = self.findChild(QPushButton, 'helpButton')
        if self.helpButton:
            self.helpButton.clicked.connect(self.open_help)
        else:
            logger.error('helpButton not found in settings.ui')

    def open_converter(self):
        subprocess.Popen([f'{self.PATH}/cw-CSV-import.exe'])
        logger.info('打开转换器')

    def open_help(self):
        subprocess.Popen([f'{self.PATH}/help.exe'])
        logger.info('需要帮助')

# 菜单类
class Menu:
    def __init__(self):
        # 初始化 Settings 类，并传递正确的 plugin_path 参数
        self.settings = Settings(plugin_path='/d:/Work/1.1.7-b4/plugins/Class-Widget-CSV-import', parent=None)

    def show_settings(self):
        self.settings.show()

# 初始化并显示菜单
if __name__ == "__main__":
    menu = Menu()
    menu.show_settings()
