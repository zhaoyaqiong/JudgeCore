
# JudgeCore 沙箱判题引擎

支持运行用户代码并隔离系统调用，限制时间、内存、输出等资源，输出用户程序运行结果。

## 构建方式

```bash
mkdir build && cd build
cmake ..
make
```

## 运行方式

```bash
./judgecore --config config.json
```

## 支持特性

- CPU 时间限制
- 内存限制
- 文件输出限制
- 系统调用过滤（seccomp）
- 输出可选返回
- 可设置用户和组权限
- 通过 JSON 配置传入参数
- 日志打印
- 动态库 + CLI 双接口
