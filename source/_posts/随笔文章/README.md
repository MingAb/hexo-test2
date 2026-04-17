---
title: REA1111====DME
---

# 🤖 ROCH Robot Workspace (`roch_ws`)

## 📖 项目简介

本项目是 ROCH 移动机器人的核心 ROS 工作空间。集成了机器人的底层驱动、传感器数据采集，以及完整的 2D/3D 建图、定位与自动导航功能。系统基于 Livox MID-360 固态激光雷达，采用 FAST-LIO 算法进行高精度 3D SLAM，并深度融合了 `move_base` 实现稳定的 2D 贴地避障导航。

## 🛠️ 系统与环境依赖

- **操作系统**: Ubuntu 20.04
- **ROS 版本**: ROS 1 Noetic
- **核心硬件**: ROCH 机器人底盘、Livox MID-360 三维激光雷达
- **关键依赖库**:
  - PCL (Point Cloud Library)
  - Livox-SDK2
  - `livox_ros_driver2`
  - `octomap_server` & `pointcloud_to_laserscan`

------

## 📂 核心功能模块

### 1. ⚙️ 驱动模块 (Driver)

- `roch_bringup`: 包含机器人底盘核心启动节点、状态发布以及集成启动脚本。
- `livox_ros_driver2`: Livox 官方雷达驱动，负责输出高频自定义格式点云。

### 2. 🗺️ 2D 建图与导航 (2D Pipeline)

- 采用传统 2D 激光雷达或由 3D 降维转换的 `/scan` 数据进行处理。
- `roch_navigation`: 包含 `move_base` 核心配置文件（全局/局部代价地图、DWA 局部规划器参数等）。
- `map_server`: 负责加载与发布 2D 黑白栅格地图。

### 3. 🌐 3D 建图与定位 (3D Pipeline)

- `FAST_LIO`: 核心 3D 激光里程计与建图算法。
- `fast_lio_localization`: 基于全局 `.pcd` 点云地图的 ICP 重定位与实时位姿融合模块。
- **高低维坐标系分离系统**: 独创的 TF 树架构，将 3D 点云保留在 `map_lidar` (半空雷达高度)，将 2D 导航限制在 `map` (地面)，实现完美的物理贴合。

------

## 🚀 快速启动指引

### 编译工作空间

Bash

```
cd ~/roch_ws
catkin build
source devel/setup.bash
```

### 一键启动底层驱动

启动底盘与雷达（脚本内置自动清理与延迟启动保护）：

Bash

```
./start_driver.sh true  # 启动驱动并打开 RViz 可视化
```

### 建图与保存

Bash

```
# 启动 3D 建图 (FAST-LIO + OctoMap 投影)
roslaunch roch_bringup mapping_3d.launch

# 建图完成后，运行保存脚本（同时保存 2D .yaml/.pgm 和 3D .bt 地图）
./save_3d_map.sh my_test_map
```

### 全局定位与导航

Bash

```
# 加载地图并启动定位与 move_base 导航
roslaunch roch_navigation navigation_3d.launch
```

> **提示**: 启动后请在 RViz 中使用 `2D Pose Estimate` 给出初始位姿，系统会自动消除里程计误差并完成精准重定位。

------

## 📝 更新日志 (Changelog)

### [2026-03-03] - 坐标系架构与建图逻辑重构

- **🐛 Bug Fix (地图保存脚本)**:
  - 优化了 `map_server` 保存 2D 栅格地图的逻辑。在保存由 3D 投影生成的 2D 地图时，脚本会自动将 `.yaml` 文件中姿态旋转产生的非法 `nan` 值替换为 `0.0`，彻底解决了重新加载地图时 `yaml-cpp: bad conversion` 的崩溃报错。
- **✨ Feature (建图 Launch 优化)**:
  - 重新梳理了建图阶段的 TF 树架构，添加了静态锚点 (`static_transform_publisher`)，解决了建图过程中由于坐标系未闭环导致的 2D 投影地图或底盘模型在 RViz 中消失、错位的问题。实现了建图过程中小车、3D 点云与 2D 栅格的实时完美贴合。

------

## 📌 TODO / 待办事项

- [ ] 进一步优化 DWA 局部规划器在狭窄走廊中的参数。
- [ ] 测试多目标点巡航脚本。