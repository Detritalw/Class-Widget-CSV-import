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

from PyQt5.QtWidgets import QHBoxLayout, QPushButton, QFileDialog, QLineEdit, QMessageBox
from qfluentwidgets import ImageLabel
import subprocess
import os
from PyQt5.QtCore import QTimer

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

        # 设置文件夹路径输入框的默认值
        parent_dir = os.path.abspath(os.path.join(self.PATH, "../../"))
        default_schedule_path = os.path.join(parent_dir, 'config', 'schedule')
        self.folderPathEdit = self.findChild(QLineEdit, 'nameEdit_3')
        if self.folderPathEdit:
            self.folderPathEdit.setText(default_schedule_path)
        else:
            logger.error('folderPathEdit not found in settings.ui')

        # 添加选择文件夹按钮
        self.selectFolderButton = self.findChild(QPushButton, 'pushButton_7')
        if self.selectFolderButton:
            self.selectFolderButton.clicked.connect(self.select_folder)
        else:
            logger.error('selectFolderButton not found in settings.ui')

        # 添加打开转换器按钮
        self.openButton = self.findChild(QPushButton, 'openButton')
        if self.openButton:
            self.openButton.clicked.connect(self.open_converter)
        else:
            logger.error('openButton not found in settings.ui')

        # 添加选择文件按钮
        self.selectFileButton = self.findChild(QPushButton, 'pushButton_2')
        if self.selectFileButton:
            self.selectFileButton.clicked.connect(self.select_file)
        else:
            logger.error('selectFileButton not found in settings.ui')

        # 添加转换按钮
        self.convertCsvToJsonButton = self.findChild(QPushButton, 'pushButton_4')
        if self.convertCsvToJsonButton:
            self.convertCsvToJsonButton.clicked.connect(self.convert_csv_to_json)
        else:
            logger.error('convertCsvToJsonButton not found in settings.ui')

        self.convertJsonToCsvButton = self.findChild(QPushButton, 'pushButton_5')
        if self.convertJsonToCsvButton:
            self.convertJsonToCsvButton.clicked.connect(self.convert_json_to_csv)
        else:
            logger.error('convertJsonToCsvButton not found in settings.ui')

        # 添加打开帮助按钮
        self.helpButton = self.findChild(QPushButton, 'pushButton_3')
        if self.helpButton:
            self.helpButton.clicked.connect(self.open_help)
        else:
            logger.error('helpButton not found in settings.ui')

        # 添加打开GitHub按钮
        self.githubButton = self.findChild(QPushButton, 'pushButton_6')
        if self.githubButton:
            self.githubButton.clicked.connect(self.open_github)
        else:
            logger.error('githubButton not found in settings.ui')

        # 文件路径输入框
        self.filePathEdit = self.findChild(QLineEdit, 'nameEdit_2')
        if not self.filePathEdit:
            logger.error('filePathEdit not found in settings.ui')

    def select_folder(self):
        folder_path = QFileDialog.getExistingDirectory(self, "选择文件夹", "")
        if folder_path:
            self.folderPathEdit.setText(folder_path)
            logger.info(f'选择的文件夹: {folder_path}')

    def open_converter(self):
        subprocess.Popen([f'{self.PATH}/cw-CSV-import.exe'])
        logger.info('打开转换器')

    def select_file(self):
        file_path, _ = QFileDialog.getOpenFileName(self, "选择文件", "", "All Files (*);;CSV Files (*.csv);;JSON Files (*.json)")
        if file_path:
            self.filePathEdit.setText(file_path)
            logger.info(f'选择的文件: {file_path}')

    def convert_csv_to_json(self):
        file_path = self.filePathEdit.text()
        folder_path = self.folderPathEdit.text()
        if file_path:
            with open(f'{self.PATH}/link.ini', 'w') as f:
                f.write(f'{file_path}\n{folder_path}')
            logger.info('从csv文件转换到json课表文件')
            QTimer.singleShot(2000, lambda: self.run_csv2json())
        else:
            QMessageBox.warning(self, "警告", "未填写源文件位置，无法进行转换")

    def convert_json_to_csv(self):
        file_path = self.filePathEdit.text()
        folder_path = self.folderPathEdit.text()
        if file_path:
            with open(f'{self.PATH}/link.ini', 'w') as f:
                f.write(f'{file_path}\n{folder_path}')
            logger.info('从json课表文件转换到csv文件')
            QTimer.singleShot(2000, lambda: self.run_json2csv())
        else:
            QMessageBox.warning(self, "错误", "未填写源文件位置，无法进行转换")

    def run_csv2json(self):
        subprocess.Popen([f'{self.PATH}/csv2json.exe'])

    def run_json2csv(self):
        subprocess.Popen([f'{self.PATH}/Class-Widget-CSV-import.exe'])

    def open_help(self):
        import webbrowser
        webbrowser.open('https://bloretcrew.feishu.cn/wiki/BGXsw2TTUiqvREk1QLkc7nuCnvT?from=from_copylink')
        logger.info('打开帮助页面')

    def open_github(self):
        import webbrowser
        webbrowser.open('https://github.com/Detritalw/Class-Widget-CSV-import')
        logger.info('打开GitHub页面')

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
