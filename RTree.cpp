
#include "RTree.h"

namespace hw6 {

	RTree::RTree(int maxChildren) : Tree(maxChildren), maxChildren(maxChildren) {
		if (maxChildren < 4) throw std::invalid_argument("maxChildren must be >= 4");
		root = nullptr;
	}

	// RNode 实现
	void RNode::add(RNode* child) {
		if (childrenNum < children.size())
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
				if(children[i] != nullptr) children[i]->draw();
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
		} else {
			for (int i = 0; i < childrenNum; ++i) {
				if(children[i] != nullptr) children[i]->rangeQuery(rect, features);
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
		return this;
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

		//bbox = Envelope(-74.1, -73.8, 40.6, 40.8); // 注意此行代码需要更新为features的包围盒，或根节点的包围盒
		// bbox = Envelope(std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());

		std::vector<Feature> shuffled_features = features;
		std::default_random_engine rng(std::random_device{}());
		std::shuffle(shuffled_features.begin(), shuffled_features.end(), rng);

		for (Feature f : shuffled_features) {
			insertFeature(root, f);
		}

		bbox = features[0].getEnvelope();
		for (const Feature f : features) {
			bbox = bbox.unionEnvelope(f.getEnvelope());
		}

		root->setEnvelope(bbox);

		return true;
	}

	void RTree::splitInnerNode(RNode* node) {
		if (node->isLeafNode() || node->getChildNum() <= maxChildren) return;

		std::vector<RNode*> children;
		children.reserve(node->getChildNum());
		for (int i = 0; i < node->getChildNum(); ++i) {
			children.emplace_back(node->getChildNode(i));
		}

		std::vector<std::pair<RNode*, Envelope>> entry;
		entry.reserve(children.size());
		for (const auto& child : children) {
			entry.emplace_back(child, child->getEnvelope());
		}

		size_t seed1 = 0, seed2 = 0;
		double minYCenter = std::numeric_limits<double>::infinity();
		double maxYCenter = -std::numeric_limits<double>::infinity();

		for (size_t i = 0; i < entry.size(); ++i) {
			double yCenter = (entry[i].second.getMinY() + entry[i].second.getMaxY()) / 2.0;
			if (yCenter < minYCenter) {
				minYCenter = yCenter;
				seed1 = i;
			}
			if (yCenter > maxYCenter) {
				maxYCenter = yCenter;
				seed2 = i;
			}
		}

		std::vector<RNode*> n1;
		std::vector<RNode*> n2;
		n1.emplace_back(entry[seed1].first);
		n2.emplace_back(entry[seed2].first);

		if (seed1 > seed2) std::swap(seed1, seed2);
		entry.erase(entry.begin() + seed2);
		entry.erase(entry.begin() + seed1);

		Envelope newEnvelope1 = n1[0]->getEnvelope();
		Envelope newEnvelope2 = n2[0]->getEnvelope();

		// 分配剩余的子节点到两个组
		for (const auto& childPair : entry) {
			const RNode* child = childPair.first;
			const Envelope& childEnv = childPair.second;

			double increase1 = newEnvelope1.unionEnvelope(childEnv).getArea() - newEnvelope1.getArea();
			double increase2 = newEnvelope2.unionEnvelope(childEnv).getArea() - newEnvelope2.getArea();

			if (increase1 < increase2) {
				n1.emplace_back(const_cast<RNode*>(child));
				newEnvelope1 = newEnvelope1.unionEnvelope(childEnv);
			}
			else {
				n2.emplace_back(const_cast<RNode*>(child));
				newEnvelope2 = newEnvelope2.unionEnvelope(childEnv);
			}
		}

		// 创建新的节点并分配子节点
		RNode* newNode1 = new RNode(newEnvelope1);
		RNode* newNode2 = new RNode(newEnvelope2);

		// 将 group1 的子节点添加到 newNode1
		for (const auto& child : n1) {
			newNode1->add(child);
			node->remove(child);
		}

		// 将 group2 的子节点添加到 newNode2
		for (const auto& child : n2) {
			newNode2->add(child);
			node->remove(child);
		}

		// 更新 envelopes
		newNode1->setEnvelope(newEnvelope1);
		newNode2->setEnvelope(newEnvelope2);

		// 处理父节点
		if (node->getParent() == nullptr) {
			// 如果当前节点是根节点，创建新的根
			Envelope newRootEnv = newNode1->getEnvelope().unionEnvelope(newNode2->getEnvelope());
			node->setEnvelope(newRootEnv);
			root = node;
			node->add(newNode1);
			node->add(newNode2);
		}
		else {
			// 将新节点添加到父节点
			RNode* parent = node->getParent();
			parent->add(newNode1);
			parent->add(newNode2);
			parent->remove(node);

			// 检查父节点是否需要进一步分裂
			if (parent->getChildNum() > maxChildren) {
				splitInnerNode(parent);
			}
			else {
				// 更新祖先节点的 envelopes
				Envelope updatedEnv = newNode1->getEnvelope().unionEnvelope(newNode2->getEnvelope());
				while (parent != nullptr) {
					Envelope parentEnv = parent->getEnvelope().unionEnvelope(updatedEnv);
					parent->setEnvelope(parentEnv);
					parent = parent->getParent();
				}
			}
		}
	}


	void RTree::splitLeafNode(RNode* node) {
		// 如果不是叶节点或特征数量未超出容量，则无需分裂
		if (!node->isLeafNode() || node->getFeatureNum() <= capacity) return;

		// 收集所有特征
		std::vector<Feature> features = node->getFeatures();
		std::vector<std::pair<Feature, Envelope>> entry;
		entry.reserve(features.size());
		for (const auto& feature : features) {
			entry.emplace_back(feature, feature.getEnvelope());
		}

		size_t seed1 = 0, seed2 = 0;
		double minYCenter = std::numeric_limits<double>::infinity();
		double maxYCenter = -std::numeric_limits<double>::infinity();

		for (size_t i = 0; i < entry.size(); ++i) {
			double yCenter = (entry[i].second.getMinY() + entry[i].second.getMaxY()) / 2.0;
			if (yCenter < minYCenter) {
				minYCenter = yCenter;
				seed1 = i;
			}
			if (yCenter > maxYCenter) {
				maxYCenter = yCenter;
				seed2 = i;
			}
		}

		std::vector<Feature> n1;
		std::vector<Feature> n2;
		n1.emplace_back(entry[seed1].first);
		n2.emplace_back(entry[seed2].first);

		if (seed1 > seed2) std::swap(seed1, seed2);
		entry.erase(entry.begin() + seed2);
		entry.erase(entry.begin() + seed1);

		Envelope newEnvelope1 = entry.empty() ? n1[0].getEnvelope() : n1[0].getEnvelope();
		Envelope newEnvelope2 = entry.empty() ? n2[0].getEnvelope() : n2[0].getEnvelope();

		for (const auto& featurePair : entry) {
			const Feature& feature = featurePair.first;
			const Envelope& featureEnv = featurePair.second;

			double increase1 = newEnvelope1.unionEnvelope(featureEnv).getArea() - newEnvelope1.getArea();
			double increase2 = newEnvelope2.unionEnvelope(featureEnv).getArea() - newEnvelope2.getArea();

			if (increase1 < increase2) {
				n1.emplace_back(feature);
				newEnvelope1 = newEnvelope1.unionEnvelope(featureEnv);
			}
			else {
				n2.emplace_back(feature);
				newEnvelope2 = newEnvelope2.unionEnvelope(featureEnv);
			}
		}

		RNode* newNode1 = new RNode(newEnvelope1);
		RNode* newNode2 = new RNode(newEnvelope2);

		for (const auto& feature : n1) {
			newNode1->add(feature);
			node->remove(feature);
		}

		for (const auto& feature : n2) {
			newNode2->add(feature);
			node->remove(feature);
		}

		newNode1->setEnvelope(newEnvelope1);
		newNode2->setEnvelope(newEnvelope2);

		if (node->getParent() == nullptr) {
			Envelope newRootEnv = newNode1->getEnvelope().unionEnvelope(newNode2->getEnvelope());
			node->setEnvelope(newRootEnv);
			root = node;
			node->add(newNode1);
			node->add(newNode2);
		}
		else {
			RNode* parent = node->getParent();
			parent->add(newNode1);
			parent->add(newNode2);
			parent->remove(node);

			if (parent->getChildNum() > maxChildren) {
				splitInnerNode(parent);
			}
			else {
				Envelope updatedEnv = newNode1->getEnvelope().unionEnvelope(newNode2->getEnvelope());
				while (parent != nullptr) {
					Envelope parentEnv = parent->getEnvelope().unionEnvelope(updatedEnv);
					parent->setEnvelope(parentEnv);
					parent = parent->getParent();
				}
			}
		}
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

} // namespace hw6