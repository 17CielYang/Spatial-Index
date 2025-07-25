# 如何运行

- 安装[CMake](https://cmake.org/download/)，版本大于等于3.16
  - 如果使用的 IDE 为 Visual Studio，也可用内置的 CMake
- 对于使用 VS Code 的同学，需要安装 VS Code 的 CMake 插件，并按网上教程配置环境变量、C/C++编译器等
- 直接用 IDE 打开此文件夹，一般会自动加载项目
  - 使用 Visual Studio 的同学，注意在 `项目 / hw6 project的CMake设置` 中点击”编辑JSON“，更改 `generator` 为本机适合的版本（如 "Visual Studio 16 2019 Win64" ），使用默认的 "Ninja" 作为 `generator` 可能有未知问题
  - 使用 Mac OS 或 Linux 的同学请使用 VS Code 或其他 IDE 打开
- 无需安装其他依赖，CMake 构建成功后可运行

## Geometry.cpp

```cpp
double Point::distance(const LineString *line) const;
double Point::distance(const Polygon *polygon) const;

bool Envelope::contain(const Envelope &envelope) const;
bool Envelope::intersect(const Envelope &envelope) const;
Envelope Envelope::unionEnvelope(const Envelope &envelope) const;

// 以下是附加题扩展要求
[[optional]] 
自行实现 Polygon 的内环几何数据存储，并修改 Point 到 Polygon 的欧式距离计算
[[optional]]
double LineString::distance(const LineString *line) const
[[optional]]
double LineString::distance(const Polygon *polygon) const
[[optional]] 
添加 MultiPoint、MultiLineString 和 MultiPolygon 类，同时给出正确性的证明

[[optional]]
bool Polygon::intersects(const Envelope &rect) const
```

## QuadTree.cpp

```cpp
void QuadNode::split(size_t capacity);
bool QuadTree::constructTree(const std::vector<Feature> &features);

void QuadNode::rangeQuery(const Envelope &rect, std::vector<Feature> &features);
void QuadTree::rangeQuery(const Envelope &rect, std::vector<Feature> &features);
bool QuadTree::NNQuery(double x, double y, std::vector<Feature> &features);
QuadNode *QuadNode::pointInLeafNode(double x, double y);
QuadNode *QuadTree::pointInLeafNode(double x, double y);

自行添加接口，实现 Spatial Join，输出满足空间距离条件的所有几何特征对，同时给出正确性的证明

// 以下是附加题扩展要求
[[optional]]
实现多边形数据基于距离的空间关联
[[optional]]
mode == Polygon 实现多边形数据的区域查询
[[optional]]
mode == Polygon 实现多边形数据的最邻近查询
[[optional]]
实现点、线和多边形的k最邻近几何特征查询（k-NN）查询，可以交互调整k的值
```

## QuadTreeTest.cpp

- 用于对 `QuadTree` 进行测试和性能分析

```cpp
[[optional]]
void QuadTree::analyse(); // 用于与R树进行比较，选择此扩展项需要同时实现R树的analyse()
```

## RTree.cpp


```cpp
bool hw6::RTree::constructTree(const std::vector<Feature>& features)

void hw6::RNode::rangeQuery(const Envelope & rect, std::vector<Feature>& features);
bool hw6::RTree::NNQuery(double x, double y, std::vector<Feature>& features)
RNode* hw6::RNode::pointInLeafNode(double x, double y);

自行添加接口，实现 Spatial Join，输出满足空间距离条件的所有几何特征对，同时给出正确性的证明

// 以下是附加题扩展要求
[[optional]]
实现多边形数据基于距离的空间关联
[[optional]]
mode == Polygon 实现多边形数据的区域查询
[[optional]]
mode == Polygon 实现多边形数据的最邻近查询
[[optional]]
实现点、线和多边形的k最邻近几何特征查询（k-NN）查询，可以交互调整k的值
```

## RTreeTest.cpp

- 用于对 `RTree` 进行测试和性能分析

```cpp
[[optional]]
static void hw6::RTree::analyse(); // 用于与四叉树进行比较，选择此扩展项需要同时实现四叉树的analyse()
```

## hw6.cpp


```cpp
void rangeQuery(); //精准判断去重
void NNQuery(hw6::Point p);  //精准判断去重
```

## [[optional]] BPlusTree.cpp

- 基于空间填充曲线的B+Tree，代码文件需要自己创建

```cpp
[[optional]]
均匀划分空间，生成Z-Curve或Hibert-Curve
[[optional]] //基于Z值或H值构造B+Tree 
bool hw6::BPlusTree::constructTree(const std::vector<Feature>& features) 
[[optional]]
void hw6::BPlusTree::rangeQuery(const Envelope & rect, std::vector<Feature>& features);
[[optional]]
bool hw6::BPlusTree::NNQuery(double x, double y, std::vector<Feature>& features)
[[optional]]
自行添加接口，实现 Spatial Join，输出满足空间距离条件的所有几何特征对，同时给出正确性的证明        
```

## 注意

- 代码中包含中文注释。所有中文注释都是 GB 编码的，需使用支持 GB 编码的编辑器打开。如果你在 Mac 或 Linux 上运行程序，可能需要先进行转码
- 运行时使用 四叉树 还是 R树 由有无定义宏 `USE_RTREE` 决定。`USE_RTREE` 位于 `Common.h` 中，默认为使用四叉树
