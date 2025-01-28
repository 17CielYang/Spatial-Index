
#include "RTree.h"
#include "Common.h"
#include <set>
#include <iomanip>
#include <queue>
#include <vector>
#include <cmath>

namespace hw6 {

	// RNode 实现
	void RNode::add(RNode* child) {
		if(childrenNum < children.size())
			children[childrenNum] = child;
		else
			children.push_back(child);
		child->parent = this;
		++childrenNum;
	}

	void RNode::remove(const Feature& f) {
		auto where = [&]() {
			for (auto itr = features.begin(); itr != features.end(); ++itr)
				if (itr->getName() == f.getName())
					return itr;
			}();
			features.erase(where);
			if (features.empty())
				features.shrink_to_fit(); // free memory unused but allocated
	}

	void RNode::remove(RNode* child) {
		for (int i = 0; i < childrenNum; ++i)
			if (children[i] == child) {
				--childrenNum;
				std::swap(children[i], children[childrenNum]);
				children[childrenNum] = nullptr;
				break;
			}
	}

	Feature RNode::popBackFeature() {
		auto ret = features.back();
		features.pop_back();
		return ret;
	}

	RNode* RNode::popBackChildNode() {
		--childrenNum;
		auto ret = children[childrenNum];
		children[childrenNum] = nullptr;
		return ret;
	}

	void RNode::countNode(int& interiorNum, int& leafNum) {
		if (isLeafNode()) {
			++leafNum;
		}
		else {
			++interiorNum;
			for (int i = 0; i < childrenNum; ++i)
				children[i]->countNode(interiorNum, leafNum);
		}
	}

	int RNode::countHeight(int height) {
		++height;
		if (!isLeafNode()) {
			int cur = height;
			for (int i = 0; i < childrenNum; ++i)
				height = std::max(height, children[i]->countHeight(cur));
		}
		return height;
	}

	void RNode::draw() {
		if (isLeafNode()) {
			bbox.draw();
		}
		else
			for (int i = 0; i < childrenNum; ++i)
				children[i]->draw();
	}

	void RNode::rangeQuery(const Envelope& rect, std::vector<Feature>& features) {
		// Task rangeQuery
		/* TODO */
		// filter step (选择查询区域与几何对象包围盒相交的几何对象)
		// 注意R树区域查询仅返回候选集，精炼步在hw6的rangeQuery中完成
		if (!bbox.intersect(rect)) return;

		if (isLeafNode()) {
			for (Feature f : this->features) {
				if (rect.intersect(f.getEnvelope())) {
					features.push_back(f);
				}
			}
		}
		else {
			for (int i = 0; i < childrenNum; ++i) {
				children[i]->rangeQuery(rect, features);
			}
		}
	}

	RNode* RNode::pointInLeafNode(double x, double y) {
		// Task pointInLeafNode
		/* TODO */
		if (isLeafNode() && bbox.contain(x, y)) {
			return this;
		}
		else {
			for (int i = 0; i < childrenNum; ++i) {
				if (children[i]->getEnvelope().contain(x, y)) {
					return children[i]->pointInLeafNode(x, y);
				}
			}
		}
		// 注意查询点在R-Tree叶节点包围盒之外的情况，返回最小的节点
		return this;
	}

	void RNode::collectAllFeatures(std::vector<Feature>& allFeatures) {
		if (isLeafNode()) {
			allFeatures.insert(allFeatures.end(), features.begin(), features.end());
		}
		else {
			for (int i = 0; i < getChildNum(); ++i) {
				children[i]->collectAllFeatures(allFeatures);
			}
		}
	}

	void RNode::KNNQuery(double x, double y, int k, std::priority_queue<Distance, std::vector<Distance>, std::less<Distance>>& pq) {
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
				else if (dist < pq.top().dist) {
					pq.pop();                  // 替换堆顶元素
					pq.emplace(dist, &feature);
				}
			}
		}
		else {
			// 非叶节点，递归处理子节点
			for (int i = 0; i < getChildNum(); ++i) {
				if (children[i] != nullptr) {
					children[i]->KNNQuery(x, y, k, pq);
				}
			}
		}
	}

	// RTree 实现
	RTree::RTree(int maxChildren) : Tree(maxChildren), maxChildren(maxChildren) {
		if (maxChildren < 4) throw std::invalid_argument("maxChildren must be >= 4");
	}

	void RTree::countNode(int& interiorNum, int& leafNum) {
		interiorNum = leafNum = 0;
		if (root != nullptr)
			root->countNode(interiorNum, leafNum);
	}

	void RTree::countHeight(int& height) {
		height = 0;
		if (root != nullptr)
			height = root->countHeight(height);
	}

	bool RTree::constructTree(const std::vector<Feature>& features) {
		// Task RTree construction
		/* TODO */

		bbox = Envelope(std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
		for (const Feature f : features) {
			bbox = bbox.unionEnvelope(f.getEnvelope());
		}

		std::vector<Feature> shuffled_features = features;
		std::default_random_engine rng(std::random_device{}());
		std::shuffle(shuffled_features.begin(), shuffled_features.end(), rng);

		for (Feature f : shuffled_features) {
			insertFeature(root, f);
		}
		root->setEnvelope(bbox);

		return true;
	}

	void RTree::insertFeature(RNode* node, const Feature& feature) {
		if (node == nullptr && root == node) {
			root = new RNode(feature.getEnvelope());
			root->add(feature);
		}
		else {
			if (node->isLeafNode()) {
				node->add(feature);

				if (node->getFeatureNum() > capacity) {
					splitLeafNode(node);
				}
				else {
					while (node != nullptr) {
						node->setEnvelope(node->getEnvelope().unionEnvelope(feature.getEnvelope()));
						node = node->getParent();
					}
				}
			}
			else {
				RNode* bestChild = nullptr;
				double minIncrease = std::numeric_limits<double>::infinity();
				for (size_t i = 0; i < node->getChildNum(); ++i) {
					RNode* child = node->getChildNode(i);
					double increase = child->getEnvelope().unionEnvelope(feature.getEnvelope()).getArea() - child->getEnvelope().getArea();
					if (increase < minIncrease) {
						minIncrease = increase;
						bestChild = child;
					}
				}
				insertFeature(bestChild, feature);
			}
		}
	}

	void RTree::splitLeafNode(RNode* node) {
		if(!node->isLeafNode() || node->getFeatureNum() <= capacity) return;

		std::vector<Feature> features = node->getFeatures();
		std::vector<std::pair<Feature, Envelope>> sortedFeatures;
		for (Feature& feature : features) {
			sortedFeatures.push_back({ feature, feature.getEnvelope() });
		}

		size_t seed1 = -1, seed2 = -1;
		double minX = std::numeric_limits<double>::infinity();  // 最小X坐标
		double maxX = -std::numeric_limits<double>::infinity(); // 最大X坐标
		for (size_t i = 0; i < sortedFeatures.size(); ++i) {

			double X = (sortedFeatures[i].second.getMinY() + sortedFeatures[i].second.getMaxY()) / 2;
			if (X < minX) {
				minX = X;//都是用水平方向的最小吗？
				seed1 = i;
			}
			if (X > maxX) {
				maxX = X;
				seed2 = i;
			}
		}

		std::vector<Feature> group1, group2;
		group1.push_back(sortedFeatures[seed1].first);
		group2.push_back(sortedFeatures[seed2].first);

		if (seed1 < seed2) {
			sortedFeatures.erase(sortedFeatures.begin() + seed1);
			sortedFeatures.erase(sortedFeatures.begin() + seed2 - 1);
		}
		else {
			sortedFeatures.erase(sortedFeatures.begin() + seed2);
			sortedFeatures.erase(sortedFeatures.begin() + seed1 - 1);
		}

		Envelope group1Envelope = features[seed1].getEnvelope();
		Envelope group2Envelope = features[seed2].getEnvelope();

		for (const auto& feature : sortedFeatures) {
			double increase1 = group1Envelope.unionEnvelope(feature.first.getEnvelope()).getArea() - group1Envelope.getArea();
			double increase2 = group2Envelope.unionEnvelope(feature.first.getEnvelope()).getArea() - group2Envelope.getArea();

			if (increase1 < increase2) {
				group1.push_back(feature.first);
			}
			else {
				group2.push_back(feature.first);
			}
		}


		RNode* newNode1 = new RNode(group1Envelope);
		RNode* newNode2 = new RNode(group2Envelope);

		for (const auto& feature : group1) {
			newNode1->add(feature);
			node->remove(feature);
			group1Envelope = group1Envelope.unionEnvelope(feature.getEnvelope());
		}
		for (const auto& feature : group2) {
			newNode2->add(feature);
			node->remove(feature);
			group2Envelope = group2Envelope.unionEnvelope(feature.getEnvelope());
		}
		newNode1->setEnvelope(group1Envelope);
		newNode2->setEnvelope(group2Envelope);

		if (node->getParent() == nullptr) {
			node->setEnvelope(node->getEnvelope().unionEnvelope(newNode1->getEnvelope()).unionEnvelope(newNode2->getEnvelope()));
			root = node;
			node->add(newNode1);
			node->add(newNode2);
		}
		else {
			node->getParent()->add(newNode1);
			node->getParent()->add(newNode2);
			RNode* parent = node->getParent();
			node->getParent()->remove(node);

			if (parent->getChildNum() > maxChildren) {
				splitNonLeafNode(parent);
			}
			else {
				while (parent != nullptr) {
					parent->setEnvelope(parent->getEnvelope().unionEnvelope(newNode1->getEnvelope()).unionEnvelope(newNode2->getEnvelope()));
					parent = parent->getParent();
				}
			}
		}
	}

	void RTree::splitNonLeafNode(RNode* node) {
		if (node->isLeafNode() || node->getChildNum() <= maxChildren) return;

		std::vector<RNode*> children;
		for (int i = 0; i < node->getChildNum(); ++i) {
			children.push_back(node->getChildNode(i));
		}
		std::vector<std::pair<RNode*, Envelope>> sortedChildren;
		for (const auto& child : children) {
			sortedChildren.push_back({ child, child->getEnvelope() });
		}

		size_t seed1 = -1, seed2 = -1;
		double minX = std::numeric_limits<double>::infinity();  // 最小X坐标
		double maxX = -std::numeric_limits<double>::infinity(); // 最大X坐标
		for (size_t i = 0; i < sortedChildren.size(); ++i) {

			double X = (sortedChildren[i].second.getMinY() + sortedChildren[i].second.getMaxY()) / 2;
			if (X < minX) {
				minX = X;
				seed1 = i;
			}
			if (X > maxX) {
				maxX = X;
				seed2 = i;
			}
		}

		std::vector<RNode*> group1, group2;
		group1.push_back(sortedChildren[seed1].first);
		group2.push_back(sortedChildren[seed2].first);
		if (seed1 < seed2) {
			sortedChildren.erase(sortedChildren.begin() + seed1);
			sortedChildren.erase(sortedChildren.begin() + seed2 - 1);
		}
		else {
			sortedChildren.erase(sortedChildren.begin() + seed2);
			sortedChildren.erase(sortedChildren.begin() + seed1 - 1);
		}

		Envelope group1Envelope = group1[0]->getEnvelope();
		Envelope group2Envelope = group2[0]->getEnvelope();

		for (const auto& child : sortedChildren) {
			double increase1 = group1Envelope.unionEnvelope(child.first->getEnvelope()).getArea() - group1Envelope.getArea();
			double increase2 = group2Envelope.unionEnvelope(child.first->getEnvelope()).getArea() - group2Envelope.getArea();

			if (increase1 < increase2) {
				group1.push_back(child.first);
			}
			else {
				group2.push_back(child.first);
			}
		}


		RNode* newNode1 = new RNode(group1Envelope);
		RNode* newNode2 = new RNode(group2Envelope);

		for (const auto& child : group1) {
			newNode1->add(child);
			node->remove(child);
			group1Envelope = group1Envelope.unionEnvelope(child->getEnvelope());
		}
		for (const auto& child : group2) {
			newNode2->add(child);
			node->remove(child);
			group2Envelope = group2Envelope.unionEnvelope(child->getEnvelope());
		}
		newNode1->setEnvelope(group1Envelope);
		newNode2->setEnvelope(group2Envelope);

		if (node->getParent() == nullptr) {
			node->setEnvelope(node->getEnvelope().unionEnvelope(newNode1->getEnvelope()).unionEnvelope(newNode2->getEnvelope()));
			root = node;
			node->add(newNode1);
			node->add(newNode2);
		}
		else {
			node->getParent()->add(newNode1);
			node->getParent()->add(newNode2);
			RNode* parent = node->getParent();
			node->getParent()->remove(node);

			if (parent->getChildNum() > maxChildren) {
				splitNonLeafNode(parent);
			}
			else {
				while (parent != nullptr) {
					parent->setEnvelope(parent->getEnvelope().unionEnvelope(newNode1->getEnvelope()).unionEnvelope(newNode2->getEnvelope()));
					parent = parent->getParent();
				}
			}
		}
	}

	void RTree::rangeQuery(const Envelope& rect, std::vector<Feature>& features) {
		features.clear();
		if (root != nullptr)
			root->rangeQuery(rect, features);
	}

	bool RTree::NNQuery(double x, double y, std::vector<Feature>& features) {
		if (!root || !(root->getEnvelope().contain(x, y)))
			return false;
		features.clear();
		// Task NNQuery 
		/* TODO */

		// filter step
		// (使用maxDistance2Envelope函数，获得查询点到几何对象包围盒的最短的最大距离，然后区域查询获得候选集)

		const Envelope& envelope = root->getEnvelope();
		double minDist = std::max(envelope.getWidth(), envelope.getHeight());

		// 注意R树邻近查询仅返回候选集，精炼步在hw6的NNQuery中完成

		RNode* pNode = root->pointInLeafNode(x, y);
		if (pNode->isLeafNode()) {
			for (size_t i = 0; i < pNode->getFeatureNum(); i++) {
				minDist = std::min(minDist, pNode->getFeature(i).maxDistance2Envelope(x, y));
			}
		}
		else {
			for (int i = 0; i < pNode->getChildNum(); i++) {
				RNode* pChild = pNode->getChildNode(i);
				for (size_t j = 0; j < pChild->getFeatureNum(); j++) {
					minDist = std::min(minDist, pChild->getFeature(j).maxDistance2Envelope(x, y));
				}
			}
		}

		Envelope rect(x - minDist, x + minDist, y - minDist, y + minDist);

		rangeQuery(rect, features);

		return true;
	}

	void RTree::getAllFeatures(std::vector<Feature>& features) {
		if (root == nullptr)
			return;
		root->collectAllFeatures(features);
	}

	bool RTree::KNNQuery(double x, double y, int k, std::vector<Feature>& result) {
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

	//	if (!root || !(root->getEnvelope().contain(x, y)))
	//		return false;
	//	features.clear();

	//	std::priority_queue<Distance, std::vector<Distance>, std::greater<Distance>> pq;
	//	if(!root->isLeafNode())
	//		pq.push(Distance(root, root->getEnvelope().maxDistance2Envelope(x, y)));

	//	double maxdist = -std::numeric_limits<double>::infinity();
	//	while (!pq.empty() && features.size() < k) {
	//		Distance top = pq.top();
	//		pq.pop();
	//		// 如果是叶节点
	//		if (top.pNode->isLeafNode()) {
	//			// 遍历叶节点的所有元素，选择距离最短的k个元素
	//			for (size_t i = 0; i < top.pNode->getFeatureNum(); i++) {
	//				Feature feature = top.pNode->getFeature(i);
	//				double distance = feature.distance(x, y); // 假设Feature类有一个获取距离的方法

	//				// 如果features的大小小于k，直接添加
	//				if (features.size() < k) {
	//					bool hasFeature = false;
	//					for (auto& f : features) {
	//						if (f.getName() == feature.getName()) {
	//							hasFeature = true;
	//							break;
	//						}
	//					}
	//					if (!hasFeature) {
	//						features.push_back(feature);
	//					}
	//				}
	//				else {
	//					// 找到features中距离最远的元素
	//					auto max_distance_feature_iter = std::max_element(features.begin(), features.end(),
	//						[x, y](const Feature& a, const Feature& b) {
	//							return a.distance(x, y) < b.distance(x, y);
	//						});

	//					// 如果当前特征的距离小于最远距离的元素，替换它
	//					if (distance < (*max_distance_feature_iter).distance(x, y)) {
	//						*max_distance_feature_iter = feature;
	//					}
	//				}
	//				maxdist = std::max(maxdist, distance);
	//			}
	//		}
	//		else {
	//			// 遍历非叶节点的所有子节点
	//			for (int i = 0; i < top.pNode->getChildNum(); i++) {
	//				// 计算子节点的距离
	//				double dist = top.pNode->getChildNode(i)->getEnvelope().maxDistance2Envelope(x, y);
	//				// 将子节点加入优先队列
	//				pq.push(Distance(top.pNode->getChildNode(i), dist));
	//			}
	//		}
	//	}
	//	//std::cout << maxdist << std::endl;
	//	Envelope rect(x - maxdist, x + maxdist, y - maxdist, y + maxdist);
	//	features.clear();
	//	rangeQuery(rect, features);
	//	return true;
	//}



	/*void RTree::getLeafNodes(std::vector<RNode*>& leafNodes) {
		std::queue<RNode*> q;
		q.push(root);
		while (!q.empty()) {
			RNode* node = q.front();
			q.pop();
			if (node->isLeafNode()) {
				leafNodes.push_back(node);
			}
			else {
				for (int i = 0; i < node->getChildNum(); ++i) {
					q.push(node->getChildNode(i));
				}
			}
		}
	}*/

	void RNode::leafNodesQuery(const Envelope& rect, std::vector<RNode*>& leafNodes) {
		if (isLeafNode()) {
			if (rect.intersect(getEnvelope())) {
				leafNodes.push_back(this);
			}
		}
		else {
			for (int i = 0; i < childrenNum; ++i) {
				children[i]->leafNodesQuery(rect, leafNodes);
			}
		}
	}

	void RTree::leafNodesQuery(const Envelope& rect, std::vector<RNode*>& leafNodes) {
		if (root != nullptr) {
			root->leafNodesQuery(rect, leafNodes);
		}
	}

	void RTree::spatialJoin(double distance, RTree* tree, std::vector<std::pair<Feature, Feature>>& result, int mode) {
		//RTree* pTree = dynamic_cast<RTree*>(tree);
		std::vector<Feature> allFeature,features;
		getAllFeatures(allFeature);
		std::cout << allFeature.size() << std::endl;
		tree->getAllFeatures(features);

		for (auto& fb : features) {
			double dist = std::numeric_limits<double>::infinity();
			for (auto& fa : allFeature) {
				if (mode == POINTJOINLINE)
					dist = static_cast<const Point*>(fa.getGeom())->distance(static_cast<const LineString*>(fb.getGeom()));
				else if (mode == LINEJOINPOINT)
					dist = static_cast<const LineString*>(fa.getGeom())->distance(static_cast<const Point*>(fb.getGeom()));

				if (dist <= distance) {
					result.push_back(std::make_pair(fa, fb));
				}

			}
		}
		std::cout << "结果如下, 一共找到:" << result.size() << std::endl;

	}


} // namespace hw6