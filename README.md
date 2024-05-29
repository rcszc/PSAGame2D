
# PSAGame2D
> PomeloStar 2D游戏开发框架.

__当前版本:__ 先行版 v20240530

> 先行版暂无文档, 其实就是工地hhh🙂, 一堆模块还处于开发阶段. ( 希望别弃坑了~~前面还有一堆坑~~

- 图形引擎基于 OpenGL4.6
- 物理引擎基于 Box2D
- 音频混响模块基于 BASS
- 设计理念使用大量描述符(DESC)避免不必要的继承重写, 简化难度
- 框架内置多级背景处理以及多级后期处理着色器组
- 主要用于交流和学习

__开发进度:__ ███████░░░ 65%
> FrameworkCore Files: 62, LoC: 8600
---

## PSAG-ARCH 架构

### 框架层级 & 渲染架构

<p align="center">
  <img src="PSAGameDesigns/EngineLayerArch.png" style="width:45%"/>
  <span style="display:inline-block; width:0%;"></span>
  <img src="PSAGameDesigns/RendererArch.png" style="width:51%;"/>
</p>

### 顶点组管理器架构

<p align="center">
  <img src="PSAGameDesigns/VertexGroupsArch.png"/>
</p>

> 架构未完成, 可能还会有微调

## 开发中ing... DebugGUI

<img src="PSAGameDesigns/PSAGame2Dv20240529.0408.png"/>