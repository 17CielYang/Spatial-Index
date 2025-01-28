#include "QuadTree.h"
#include "Common.h"
#include <set>
#include <iomanip>
#include <queue>
#include <vector>
#include <cmath>

extern void uniqueFeatures(std::vector<hw6::Feature>& features);
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

	void QuadNode::collectAllFeatures(std::vector<Feature>& allFeatures) {
		if (isLeafNode()) {
			// 将当前节点的所有特征加入结果
			allFeatures.insert(allFeatures.end(), features.begin(), features.end());
		}
		else {
			// 递归收集子节点的特征
			for (int i = 0; i < 4; ++i) {
				children[i]->collectAllFeatures(allFeatures);
			}
		}
	}

	void QuadNode::KNNQuery(double x, double y, int k, std::priority_queue<Distance, std::vector<Distance>, std::less<Distance>>& pq) {
		// 计算当前节点的包围盒到查询点的最小距离
		double distToNode = bbox.minDistance(x, y);

		// 如果堆中已有 k 个元素，且当前节点包围盒的距离大于堆中最大距离，剪枝
		if (pq.size() == k && distToNode > pq.top().dist) {
			return;
		}
		// 如果是叶节点，处理特征
		if (isLeafNode()) {
			for (const Feature& feature : features) {
				double dist = feature.distance(x, y); // 计算特征到查询点的距离
				if (pq.size() < k) {
					pq.emplace(dist, &feature); // 堆未满，直接插入
				}
				else if (dist <= pq.top().dist) {
					pq.pop();                  // 替换堆顶元素
					pq.emplace(dist, &feature);
				}
			}
		}
		else {
			// 非叶节点，递归处理子节点
			for (int i = 0; i < 4; ++i) {
				if (children[i] != nullptr) {
					children[i]->KNNQuery(x, y, k, pq);
				}
			}
		}
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
		//std::cout <<"数据添加成功:"<<features.size() << std::endl;
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
	}
	bool QuadTree::NNQuery(double x, double y, std::vector<Feature>& features) {
		if (!root || !(root->getEnvelope().contain(x, y)))
			return false;
		QuadNode* leafNode = root->pointInLeafNode(x, y);
		if (!leafNode) {
			return false;
		}
		// filter step
		// (使用maxDistance2Envelope函数，获得查询点到几何对象包围盒的最短的最大距离，然后区域查询获得候选集)
		const Envelope& envelope = root->getEnvelope();
		double minDist = std::max(envelope.getWidth(), envelope.getHeight());
		Envelope queryRect;
		
		if (leafNode->getFeatureNum() == 0) {
			queryRect = envelope;
		}
		else {
			for (size_t i = 0; i < leafNode->getFeatureNum(); ++i) {
				const Feature& feature = leafNode->getFeature(i);  // 使用引用避免拷贝
				double dist = feature.maxDistance2Envelope(x, y);  // 计算点到包围盒的最大距离
				if (dist < minDist) {
					minDist = dist;
				}
			}
			queryRect = Envelope(x - minDist, x + minDist, y - minDist, y + minDist);
		}
		//如果当前叶节点没有feature，则查询范围是整个包围盒
		root->rangeQuery(queryRect, features);
		return true;
	}
	// 自定义比较器
	/*struct PairComparator {
		bool operator()(const std::pair<Feature, Feature>& a, const std::pair<Feature, Feature>& b) const {
			if (a.first.getName()!=  b.first.getName()) return a.first < b.first;
			return a.second < b.second;
		}
	};*/

	void QuadTree::getAllFeatures(std::vector<Feature>& features) {
		if (root == nullptr)
			return;
		root->collectAllFeatures(features);
	}

	void QuadTree::spatialJoin(double distance, std::vector<Feature> features, std::vector<std::pair<Feature, Feature>>& result,int mode) {
		if (root == nullptr || features.size()==0) {
			return;
		}
		std::vector<Feature> allFeaturesA;
		root->collectAllFeatures(allFeaturesA);
		uniqueFeatures(allFeaturesA);
		std::cout << allFeaturesA.size() << std::endl;
		std::cout << features.size() << std::endl;
		int a = 0;
		for (auto& fb : features) {
			//Envelope e = Envelope(fb.getEnvelope().getMinX() - distance, fb.getEnvelope().getMinY() - distance, fb.getEnvelope().getMaxX() + distance, fb.getEnvelope().getMaxY() + distance);
			//std::vector<Feature> candidateFeatures;
			//rangeQuery(e, candidateFeatures);
			for (auto& fa : allFeaturesA) {
				//double dist = -std::numeric_limits<double>::infinity();
				if (mode == POINTJOINLINE)
				{
					Envelope e = fa.getEnvelope().expand(distance);
					if (fb.getGeom()->intersects(e))
						result.push_back(std::make_pair(fa, fb));
				}
				else if (mode == LINEJOINPOINT|| mode == POLYGONJOINPOINT) {
					Envelope e = fb.getEnvelope().expand(distance);
					if (fa.getGeom()->intersects(e))
						result.push_back(std::make_pair(fa, fb));
				}

			}
		}
		std::cout << "结果如下, 一共找到:" << result.size()<< std::endl;
	}

	bool QuadTree::KNNQuery(double x, double y, int k, std::vector<Feature>& result) {
		if (!root || !(root->getEnvelope().contain(x, y)))
			return false;
		result.clear();
		std::priority_queue<Distance, std::vector<Distance>, std::less<Distance>> pq;//最小堆

		root->KNNQuery(x, y, k, pq);

		// 将堆中的元素转换为结果
		while (!pq.empty()) {
			result.push_back(*pq.top().feature);
			pq.pop();
		}
		std::cout << "knn查询完成" << std::endl;
		return true;
	}

	
	void QuadTree::draw() {
		if (root)
			root->draw();
	}


	/*其他结构定义*/
} // namespace hw6