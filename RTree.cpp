
#include "RTree.h"


namespace hw6 {

	RNode::~RNode() {
		for (auto child : children) {
			delete child;
		}
		children.clear();
		features.clear();
	}

	// RNode 实现
	void RNode::add(RNode* child) {
		children.push_back(child);
		child->parent = this;
		++childrenNum;
		bbox = bbox.unionEnvelope(child->getEnvelope()); // 更新父节点的bbx
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
		auto &ret = features.back();
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
			std::cout << "Drawing leaf node success" << std::endl; // 调试输出
		}
		else {
			for (int i = 0; i < childrenNum; ++i) {
				children[i]->draw();
			}
			std::cout << "Drawing internal node success" << std::endl; // 调试输出
		}
	}

	void RNode::rangeQuery(const Envelope& rect, std::vector<Feature>& result) {
		// Task rangeQuery
		// filter step (选择查询区域与几何对象包围盒相交的几何对象)
		// 注意R树区域查询仅返回候选集，精炼步在hw6的rangeQuery中完成
		/* TODO */
		if (!bbox.intersect(rect)) return;
		if (isLeafNode()) { // 叶子节点
			for (Feature feature : features) { // 遍历当前节点的成员features
				if (feature.getEnvelope().intersect(rect)) {
					result.push_back(feature);
				}
			}
		} else { // 内部节点
			for (int i = 0; i < childrenNum; ++i) {
				if (children[i] != nullptr)
					children[i]->rangeQuery(rect, result);
			}
		}
	}

	RNode* RNode::pointInLeafNode(double x, double y) {
		// Task pointInLeafNode
		/* TODO */

		if (isLeafNode() && bbox.contain(x, y)) { 
			return this; 
		} else {
			for (int i = 0; i < childrenNum; ++i) {
				if (children[i]->getEnvelope().contain(x, y)) {
					return children[i]->pointInLeafNode(x, y);
				}
			}
		}
		return this; // 查询点在R-Tree叶节点包围盒之外，返回最小节点
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
		if (features.empty()) return false;

		bbox = features[0].getEnvelope();
		for (const Feature f : features) {
			bbox = bbox.unionEnvelope(f.getEnvelope());
		}
		root->setEnvelope(bbox); // 设置根节点包围盒为所有feature的并包

		std::vector<Feature> shuffled_features = features; // 随机打乱
		std::default_random_engine rng(std::random_device{}());
		std::shuffle(shuffled_features.begin(), shuffled_features.end(), rng);

		for (Feature f : shuffled_features) {
			insert(f);
		}

		return true;
	}

	void RTree::insert(const Feature& f) {
		if (root == nullptr)
			root = new RNode(f.getEnvelope());
		
		RNode* leaf = chooseLeaf(root, f);
		leaf->add(f);

		if (leaf->getFeatureNum() > maxChildren) {
			std::pair<RNode*, RNode*> splitNodes = splitNode(leaf);
			AdjustTree(splitNodes.first, splitNodes.second);
		} else {
			AdjustTree(leaf, nullptr);
		}
	}

	RNode* RTree::chooseLeaf(RNode* current, const Feature& f) { // 返回一个从当前节点往下找的最佳插入的叶节点
		if (current->isLeafNode()) return current; 
		// 下面选择内部节点的最佳子节点，递归到叶节点为止
		RNode* bestChild = nullptr;
		double minEnlarge = std::numeric_limits<double>::max(); // 最小增量

		Envelope fbbx = f.getEnvelope();
		// 注意：只有根节点作为叶节点的时候，featureNum可以小于minChildren
		for (int i = 0; i < current->getChildNum(); ++i) {
			RNode* child = current->getChildNode(i);
			Envelope child_bbx = child->getEnvelope();
			Envelope merged = child_bbx.unionEnvelope(fbbx);
			double oldArea = child_bbx.getArea();
			double newArea = merged.getArea();
			double enlarge = newArea - oldArea;

			if (enlarge <= minEnlarge) {
				minEnlarge = enlarge;
				bestChild = child;
			}
		}
		// make sure that bestChild is not nullptr
		if (bestChild == nullptr) {
			throw std::runtime_error("No suitable child found during chooseLeaf.");
		}
		return chooseLeaf(bestChild, f);
	}

	std::pair<RNode*, RNode*> RTree::splitNode(RNode* node) {


		if (node->isLeafNode()) { // 分裂叶子节点
			//if (node->getFeatureNum() <= 1) { // 无法分裂，返回原节点和nullptr
			//	return{ node, nullptr };
			//}
			int seed1 = 0, seed2 = 1;
			double maxD = std::numeric_limits<double>::lowest(); // 记录相距最大距离
			for (int i = 0; i < node->getFeatureNum(); ++i) { // 找到相距最大的两个种子的下标
				Envelope e1 = node->getFeature(i).getEnvelope();
				for (int j = i + 1; j < node->getFeatureNum(); ++j) {
					Envelope e2 = node->getFeature(j).getEnvelope();
					double d = e1.unionEnvelope(e2).getArea() - e1.getArea() - e2.getArea();
					if (d > maxD) {
						maxD = d;
						seed1 = i;
						seed2 = j;
					}
				}
			}

			Feature seedFeature1 = node->getFeature(seed1);
			Feature seedFeature2 = node->getFeature(seed2);

			node->setEnvelope(seedFeature1.getEnvelope()); // 重置node的包围盒为seed1的包围盒
			RNode* newNode = new RNode(seedFeature2.getEnvelope());
			newNode->add(seedFeature2);
			node->remove(seedFeature2);

			int i = 0;

			// 分配剩余条目
			while (node->getFeatureNum() > (maxChildren/2)) {
				Feature f = node->getFeature(i);
				Envelope e = f.getEnvelope();

				Envelope join1 = node->getEnvelope().unionEnvelope(e);
				Envelope join2 = newNode->getEnvelope().unionEnvelope(e);
				double d1 = join1.getArea() - node->getEnvelope().getArea() - e.getArea(); // 在node中的包围盒面积改变量
				double d2 = join2.getArea() - newNode->getEnvelope().getArea() - e.getArea(); 

				if (d1 <= d2) { // 如果在node中增量小，就保留在原位不动
					node->setEnvelope(join1);
					i++;
				} else if (d2 < d1) {
					newNode->add(f);
					node->deleteFeature(f);
				}
			}
			return { node, newNode };
		}
		else { // 分裂内部节点
			int seed1 = 0, seed2 = 1;
			double maxD = std::numeric_limits<double>::lowest(); // 记录相距最大距离
			for (int i = 0; i < node->getChildNum(); ++i) {
				for (int j = i + 1; j < node->getChildNum(); ++j) {
					Envelope e1 = node->getChildNode(i)->getEnvelope();
					Envelope e2 = node->getChildNode(j)->getEnvelope();
					double d = e1.unionEnvelope(e2).getArea() - e1.getArea() - e2.getArea();
					if (d > maxD) {
						maxD = d;
						seed1 = i;
						seed2 = j;
					}
				}
			}

			node->setEnvelope(node->getChildNode(seed1)->getEnvelope()); // 重置节点包围盒为childNode(seed1)的包围盒
			RNode* newNode = new RNode(node->getChildNode(seed2)->getEnvelope());
			newNode->add(node->getChildNode(seed2));
			node->remove(node->getChildNode(seed2));
			int i = 0;

			// 分配剩余子节点
			while (node->getChildNum() > 0) {
				RNode* child = node->getChildNode(i);
				Envelope e = child->getEnvelope();

				Envelope j1 = node->getEnvelope().unionEnvelope(e);
				Envelope j2 = newNode->getEnvelope().unionEnvelope(e);

				double d1 = j1.getArea() - node->getEnvelope().getArea() - child->getEnvelope().getArea();
				double d2 = j2.getArea() - newNode->getEnvelope().getArea() - child->getEnvelope().getArea();

				if (d1 <= d2) {
					node->setEnvelope(j1);
					i++;
				}
				else if (d2 < d1) {
					newNode->add(child);
					node->deleteChild(child);
				}
			}

			return { node, newNode };
		}
	}

	void RTree::AdjustTree(RNode* node, RNode* newNode) {
		RNode* current_Node = node;
		RNode* split_Node = newNode;

		while (current_Node != nullptr) {
			RNode* parent = current_Node->getParent();

			if (parent == nullptr) {
				// currentNode 是根节点
				if (split_Node != nullptr) {
					// 创建新的根节点，其bbox为 currentNode 和 splitNode 的并集
					Envelope newRootBbox = current_Node->getEnvelope().unionEnvelope(split_Node->getEnvelope());
					RNode* newRoot = new RNode(newRootBbox);

					newRoot->add(current_Node);
					newRoot->add(split_Node);

					// 更新树的根指针
					root = newRoot;
				}
				break; // 到达根节点，调整完成
			}

			if (split_Node != nullptr) {
				// 将 split_Node 添加到父节点
				parent->add(split_Node);
				std::cout << "Added split node to parent with bbox" << std::endl;

				// 检查父节点是否需要分裂
				if (parent->getChildNum() > maxChildren) {
					// 分裂父节点
					std::pair<RNode*, RNode*> splitNodes = splitNode(parent);
					current_Node = splitNodes.first;
					split_Node = splitNodes.second;
					std::cout << "Parent node split into two nodes." << std::endl;
				}
				else {
					break; // 父节点不需要分裂，调整完成
				}
			}
			else {
				break; // 没有分裂，调整完成
			}
		}
	}

	void RTree::rangeQuery(const Envelope& rect, std::vector<Feature>& features) {
		features.clear();
		if (root != nullptr)
			root->rangeQuery(rect, features);
	}

	bool RTree::NNQuery(double x, double y, std::vector<Feature>& features) {
		features.clear();
		// Task NNQuery 
		/* TODO */

		// filter step
		// (使用maxDistance2Envelope函数，获得查询点到几何对象包围盒的最短的最大距离，然后区域查询获得候选集)

		// 注意R树邻近查询仅返回候选集，精炼步在hw6的NNQuery中完成

		return false;
	}

} // namespace hw6