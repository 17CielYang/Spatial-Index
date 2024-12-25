#include "QuadTree.h"
#include <set>

namespace hw6 {

	/*
	 * QuadNode
	 */
	void QuadNode::split(size_t capacity) {
		for (int i = 0; i < 4; ++i) {
			delete children[i];
			children[i] = nullptr;
		}
		// Task construction
		// TODO
		double midX = (bbox.getMinX() + bbox.getMaxX()) / 2;
		double midY = (bbox.getMinY() + bbox.getMaxY()) / 2;

		children[0]= new QuadNode(Envelope(bbox.getMinX(), midX, midY, bbox.getMaxY())); // 左上
		children[1]= new QuadNode(Envelope(midX, bbox.getMaxX(), midY, bbox.getMaxY())); // 右上
		children[2] = new QuadNode(Envelope(bbox.getMinX(), midX, bbox.getMinY(), midY)); // 左下
		children[3] = new QuadNode(Envelope(midX, bbox.getMaxX(), bbox.getMinY(), midY)); //右下
		
		//为子节点添加父节点的feature
		for (const Feature& feature : features){
			for (int j = 0; j < 4; j++) {
				if (children[j]->bbox.intersect(feature.getEnvelope())) {
					children[j]->features.push_back(feature);
				}
			}
		}
		features.clear();
		for (int i = 0; i < 4; ++i) {
			if (children[i]->features.size() > capacity) {
				children[i]->split(capacity);
			}
		}

	}

	void QuadNode::countNode(int& interiorNum, int& leafNum) {
		if (isLeafNode()) {
			++leafNum;//最末端的叶节点
		}
		else {
			++interiorNum;//有子节点的内部节点
			for (int i = 0; i < 4; ++i)
				children[i]->countNode(interiorNum, leafNum);
		}
	}

	int QuadNode::countHeight(int height) {
		++height;
		if (!isLeafNode()) {
			int cur = height;
			for (int i = 0; i < 4; ++i) {
				height = std::max(height, children[i]->countHeight(cur));
			}
		}
		return height;
	}

	void QuadNode::rangeQuery(const Envelope& rect, std::vector<Feature>& result) {
		//result本来是features，但是这和Quadnode的变量重合了？
		if (!bbox.intersect(rect))
			return;
		if (isLeafNode()) {
			for (const Feature& feature : features) {
				//Geometry属性有intersects方法，但是还没实现
				if (feature.getEnvelope().intersect(rect))
					result.push_back(feature);
			}
			//std::cout << "成功获取候选feature\n";
		}
		else {
			for (int i = 0; i < 4; i++) {
				children[i]->rangeQuery(rect, result);
			}
		}
		// Task range query
		// TODO
	}

	QuadNode* QuadNode::pointInLeafNode(double x, double y) {
		if (!bbox.contain(x, y)) {
			return nullptr;
		}
		// Task NN query
		// TODO
		if (this->isLeafNode()) {
			std::cout << "find success" << std::endl;
			return this;
		}
		//递归找到包含(x,y)的末端叶节点
		for (int i = 0; i < 4; i++) {
			if (children[i]->bbox.contain(x, y)) {
				QuadNode* leafNode = children[i]->pointInLeafNode(x, y);
				if (leafNode != nullptr) {
					return leafNode;
				}
			}
		}
		return nullptr;
		
	}

	void QuadNode::draw() {
		if (isLeafNode()) {
			bbox.draw();
		}
		else {
			for (int i = 0; i < 4; ++i)
				children[i]->draw();
		}
	}

	/*
	 * QuadTree
	 */
	bool QuadTree::constructTree(const std::vector<Feature>& features) {
		if (features.empty())
			return false;
		// Task construction
		// TODO
		//生成features的包围盒
		double minX, maxX, minY, maxY;
		minX = features[0].getEnvelope().getMinX();
		maxX = features[0].getEnvelope().getMaxX();
		minY = features[0].getEnvelope().getMinY();
		maxY = features[0].getEnvelope().getMaxY();
		for (const Feature& feature : features) {
			const Envelope& featureBBox = feature.getEnvelope();
			minX = std::min(minX, featureBBox.getMinX());
			maxX = std::max(maxX, featureBBox.getMaxX());
			minY = std::min(minY, featureBBox.getMinY());
			maxY = std::max(maxY, featureBBox.getMaxY());
		}
		bbox = Envelope(minX, maxX, minY, maxY);
		//把数据插入Tree
		root = new QuadNode(bbox);
		root->add(features);
		if (root->getFeatureNum() > capacity) {
			root->split(capacity);
		}
		return true;
	}

	void QuadTree::countNode(int& interiorNum, int& leafNum) {
		interiorNum = 0;
		leafNum = 0;
		if (root)
			root->countNode(interiorNum, leafNum);
	}

	void QuadTree::countHeight(int& height) {
		height = 0;
		if (root)
			height = root->countHeight(0);
	}

	void QuadTree::rangeQuery(const Envelope& rect, std::vector<Feature>& features) {
		features.clear();

		// Task range query
		// TODO
		// filter step (选择查询区域与几何对象包围盒相交的几何对象)
		if (root != nullptr) {
			root->rangeQuery(rect, features);
		}
		// 注意四叉树区域查询仅返回候选集，精炼步在hw6的rangeQuery中完成
	}

	bool QuadTree::NNQuery(double x, double y, std::vector<Feature>& features) {
		if (!root || !(root->getEnvelope().contain(x, y)))
			return false;

		// Task NN query
		// TODO
		QuadNode* leafNode = root->pointInLeafNode(x, y);
		if (!leafNode) {
			return false;
		}
		// filter step
		// (使用maxDistance2Envelope函数，获得查询点到几何对象包围盒的最短的最大距离，然后区域查询获得候选集)

		const Envelope& envelope = root->getEnvelope();
		double minDist = std::max(envelope.getWidth(), envelope.getHeight());
		
		for (size_t i=0;i<leafNode->getFeatureNum();i++){
			// 假设 Feature 类有 getEnvelope 方法返回特征的边界框
			Feature feature = leafNode->getFeature(i);
			const Envelope& featureBBox = feature.getEnvelope();
			double dist = feature.maxDistance2Envelope(x, y); // 计算点到边界框的最大距离
			if (dist < minDist) {
				minDist = dist;
			}
		}
		//如果当前叶节点没有feature，则查询范围是整个包围盒
		Envelope queryRect(x - minDist, x + minDist, y - minDist, y + minDist);
		std::cout << "包围盒构建完毕" << std::endl;

		//查询几何特征的包围盒与该区域相交的几何特征（filter）
		root->rangeQuery(queryRect, features);

		// 注意四叉树邻近查询仅返回候选集，精炼步在hw6的NNQuery中完成
		return true;
	}

	void QuadTree::draw() {
		if (root)
			root->draw();
	}

} // namespace hw6
