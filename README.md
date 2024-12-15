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
由于时间分区功能未完成，所以导入到 [Class Widgets](https://github.com/Class-Widgets/Class-Widgets) 中会报错，如果想要导入成功，可以尝试 **在倒数第二个`}`后、最后一个`}`中间** 加入以下内容
```
"part": {
        "0": [
            时,
            分
        ],
        "1": [
            时,
            分
        ]
    },
    "part_name": {
        "0": "上午",
        "1": "下午"
    }
```

## 环境
||测试环境|建议|最低限度|
|:-:|:-:|:-:|:-:|
|操作系统|Windows 11|Windows 10+|未知(Maybe Windows XP?)|
|系统位数|x64|x64|未知(Maybe x86?)|

## 相关项目
[Class Widgets](https://github.com/Class-Widgets/Class-Widgets)
