# PSA Game2D Graphics STD
> ©PomeloStar Studio 2024-2025 （2024.05.12）

- __Developers:__ ```RCSZ```
- __Update:__ ```2024.08.06``` __Version:__ ```0.1.2```

---

### PSAG GL-VERT
> 规定图形底层 顶点组的 属性 / 数据结构.

```
GL_VERT_01: VEC3 (位置), VEC4 (颜色), VEC2 (纹理), VEC3 (法线)

顶点组数据大小: 48 Bytes
顶点组内存布局: [连续].12.16.8.12

GL_VERT_02: VEC3 (位置), VEC2 (纹理)

顶点组数据大小: 20 Bytes
顶点组内存布局: [连续].12.8
```

### PSAG GL-TEX
> 规定图形底层 纹理的 结构参数 / 过滤.

```
GL_TEX_01: DataFormat.UNISGNED_BYTES

纹理像素数据格式: 8位无符号整型"uint8_t"
纹理像素颜色排列: [连续].0(1,2,3...).1(1,2,3...)

GL_TEX_02: RGBA8888

纹理像素(颜色)格式: 内部统一采用单色8位二进制分量, RGBA格式, 32 Bytes
纹理像素(颜色)限值: 0,255 (0x00,0xff)

GL_TEX_03: RGB_AHPLA_FILL

RGB纹理处理方式: 重组像素数据, 填充Ahpla通道为255(0xff)

GL_TEX_04: FILTER_MODE

纹理过滤方式: 邻近过滤, 线性过滤, 各向异性过滤, Mipmap过滤

(2024.08.06) [修订] GL_TEX_02: HDR12

纹理像素(颜色)格式: 内部统一采用HDR12颜色模式, RGBA颜色通道
纹理像素(颜色)限值: 0,4095 (0x000,0xfff)
```

### PSAG GL-TEX-ATTR
> 规定图形底层 纹理的 类型.

```
GL_TEX_ATTR_01: SamplerTexture(Array)

底层纹理类型: 纹理数组, 中层管理分配纹理采样器
资源管理方式: LLRES底层管理, 图形引擎中层管理 (virtual texture)
框架纹理用途: 着色器纹理数据操作

GL_TEX_ATTR_02: ViewTexture2D

底层纹理类型: 单个纹理, 不分配纹理采样器
资源管理方式: 图形引擎中层管理句柄
框架纹理用途: 加载/捕获 图像提交UI(ImGui)显示
```

### PSAG GL-RENDER
> 规定渲染模式.

```
GL_RENDER_01: Projection.Matrix ORTHO

2D渲染采用正交视图

GL_RENDER_02: ORTHO.Layers

渲染层级(Z)顺序: 远离屏幕 -z <=> +z 靠近屏幕

内部后期处理采用Z坐标裁剪 +-10.0f (系统背景-10.0f, 后期图像处理+10.0f)
Actor渲染采用Z坐标裁剪 +- 100.0f
建议(Actor): 50.0f ~ 100.0f
建议(Actor.Brick): 1.0f ~ 45.0f
```