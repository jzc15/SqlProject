# SqlProject

小型SQL系统。

### 目录说明

* `datamanager` : 数据管理模块，定义了槽文件系统和向量文件系统

* `ddf` : 数据库/数据表描诉文件，实现了数据库/表/记录的描述类，实现了记录序列化/反序列化等接口

* `disk` : 页式文件系统，以及其他和磁盘交互的接口

* `frontend` : 前段模块，负责解析命令

* `json11` : 外部引用json模块

* `engine` : 核心引擎，负责组合各个模块，完成增删查改等各个指令

* `indices` : 索引模块，实现了B+树索引，唯一键值散列索引，可重键值散列索引

* `src` : 入口程序，每个文件代表一个可执行文件，包含main以及各个模块的单元测试

* `ui` : 网页GUI

## GUI运行说明

```sh
> sudo apt-get python3
> sudo apt-get python3-pip
> pip3 install django
> cd ui
> python3 manager.py runserver
```