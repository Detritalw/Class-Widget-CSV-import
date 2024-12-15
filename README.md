# [Class Widgets](https://github.com/Class-Widgets/Class-Widgets) CSV import
让我们用.csv表格编辑 [Class Widgets](https://github.com/Class-Widgets/Class-Widgets) 课表！将.csv文件转换为 Class Widgets Json 课表

## DEV
若您想要看到最新版本，Github上的更新并不及时，请前往 [PCFS 存储站](http://123.129.241.101:30390/%E5%85%B1%E4%BA%AB/Class-Widget-CSV-import) 查看最新存储库和实时出版的测试版本

## 特点
软件无依赖，单文件即可运行

## 功能与计划
- [x] 时间线转化
- [x] 课程表转化
- [ ] 时间分区
- [ ] ...

## 需要注意
出现文件中文格式问题，写入时间段名字时请不要使用中文，否则转换出来的文件若用utf-8格式打开是乱码，只能使用**Simplified Chinese (GBK)** gbk格式，但 [Class Widgets](https://github.com/Class-Widgets/Class-Widgets) 识别utf-8格式。
> [Class Widgets](https://github.com/Class-Widgets/Class-Widgets) 提示：
> | ERROR    | list:import_schedule:227 - 加载数据时出错: 'utf-8' codec can't decode byte 0xc9 in position 1307: invalid continuation byte

解决方法：
写入时间段名字时请不要使用中文，使用英文即可，例如AM，PM


## 环境
||测试环境|建议|最低限度|
|:-:|:-:|:-:|:-:|
|操作系统|Windows 11|Windows 10+|未知(Maybe Windows XP?)|
|系统位数|x64|x64|未知(Maybe x86?)|

## 相关项目
[Class Widgets](https://github.com/Class-Widgets/Class-Widgets)
