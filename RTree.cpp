
#include "RTree.h"
#include <algorithm>
#include <random>

namespace hw6 {

	// RNode 实现
	void RNode::add(RNode* child) {
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
		if (!bbox.intersect(rect)) {
			return;	// 当前节点的包围盒都不相交
		}
		if (isLeafNode()) { // 如果是叶节点，检查每个feature的包围盒
			for (auto& f : features) {
				if (f.getEnvelope().intersect(rect)) {
					features.push_back(f); // 如果相交就添加到候选集中
				}
			}
		} else { // 如果是内部节点，递归检查子节点
			for (int i = 0; i < childrenNum; ++i) {
				children[i]->rangeQuery(rect, features);
			}
		}

		// filter step (选择查询区域与几何对象包围盒相交的几何对象)
		// 注意R树区域查询仅返回候选集，精炼步在hw6的rangeQuery中完成
	}

	RNode* RNode::pointInLeafNode(double x, double y) {
		// Task pointInLeafNode
		/* TODO */
		if (!bbox.contain(x, y)) {
			return nullptr;
		}
		if (isLeafNode()) {
			return this;
		} else {
			for (int i = 0; i < childrenNum; ++i) {
				RNode* found = children[i]->pointInLeafNode(x, y);
				if (found != nullptr) return found;
			}
			return nullptr;
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
		if (features.empty()) return true;
		std::vector<Feature> new_features = features;
		std::random_device rd;
		std::default_random_engine rng(rd());
		std::shuffle(new_features.begin(), new_features.end(), rng);

		for (auto& f : new_features) {
			insert(f);
		}

		return true;
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
		if (!root) return false;

		// 优先级队列(distance, node)
		using QItem = std::pair<double, RNode*>;
		std::priority_queue<QItem, std::vector<QItem>, std::greater<>> pq;
		pq.push({ root->getEnvelope().minDistance(x, y), root });
		double bestDist = std::numeric_limits<double>::infinity();
		Feature nearestFeature;
		bool found = false;

		while (!pq.empty()) {
			auto [dist, node] = pq.top();
			pq.pop();
			if (dist > bestDist) continue;

			if (node->isLeafNode()) {
				for (auto& f : node->getFeatures()) {
					double d = f.distance(x, y);
					if (d < bestDist) {
						bestDist = d;
						nearestFeature = f;
						found = true;
					}
				}
			}
			else {
				for (int i = 0; i < node->getChildNum(); ++i) {
					RNode* c = node->getChildNode(i);
					double d = c->getEnvelope().minDistance(x, y);
					if (d <= bestDist) {
						pq.push({ d, c });
					}
				}
			}
		}


		// filter step
		// (使用maxDistance2Envelope函数，获得查询点到几何对象包围盒的最短的最大距离，然后区域查询获得候选集)

		// 注意R树邻近查询仅返回候选集，精炼步在hw6的NNQuery中完成

		if (found) features.push_back(nearestFeature);
		return found;
	}

	void RTree::insert(const Feature& f) {
		if (root == nullptr) {
			root = new RNode(f.getEnvelope(), maxChildren);
			root->add(f);
			return;
		}

		// 1. 找到合适的叶节点
		RNode* leaf = chooseLeaf(root, f);
		leaf->add(f);

		// 2. 如果叶节点溢出，分裂
		if ((int)leaf->getFeatureNum() > maxChildren) {
			auto [n1, n2] = splitNode(leaf);
			adjustTree(n1, n2);
		}
		else {
			// 无分裂，仅更新路径上的包围盒
			adjustTree(leaf, nullptr);
		}
	}

	RNode* RTree::chooseLeaf(RNode* node, const Feature& f) {
		// 从 root 向下，直到叶节点
		Envelope fe = f.getEnvelope();
		while (!node->isLeafNode()) {
			double minEnlarge = std::numeric_limits<double>::infinity();
			RNode* chosen = nullptr;
			for (int i = 0; i < node->getChildNum(); ++i) {
				RNode* c = node->getChildNode(i);
				double oldArea = c->getEnvelope().getArea();
				double newArea = c->getEnvelope().unionEnvelope(fe).getArea();
				double enlarge = newArea - oldArea;
				if (enlarge < minEnlarge) {
					minEnlarge = enlarge;
					chosen = c;
				}
			}
			node = chosen;
		}
		return node;
	}

	void RTree::adjustTree(RNode* n, RNode* nn) {
		// 向上调整包围盒，若出现溢出则分裂父节点
		while (n != root) {
			RNode* p = n->getParent();
			// 更新p的包围盒
			Envelope newEnv = computeMBRForChildren(p);
			p->setEnvelope(newEnv);

			if (nn != nullptr) {
				// 将nn插入p
				p->add(nn);
				if (p->getChildNum() > maxChildren) {
					auto [p1, p2] = splitNode(p);
					n = p1; nn = p2;
					continue;
				}
				else {
					nn = nullptr; // 已插入并处理完毕
				}
			}
			n = p;
		}

		// 如果回溯到根节点仍有nn需要插入，则创建新的根
		if (nn != nullptr) {
			RNode* oldRoot = root;
			RNode* newRoot = new RNode(computeMBRForChildren(oldRoot), maxChildren);
			newRoot->add(oldRoot);
			newRoot->add(nn);
			Envelope newEnv = computeMBRForChildren(newRoot);
			newRoot->setEnvelope(newEnv);
			root = newRoot;
		}
		else {
			// 单纯更新根包围盒
			if (root != nullptr) {
				Envelope rootEnv = computeMBRForChildren(root);
				root->setEnvelope(rootEnv);
			}
		}
	}

	std::pair<RNode*, RNode*> RTree::splitNode(RNode* n) {
		// 根据n是否为叶节点，选择分裂目标
		if (n->isLeafNode()) {
			// 基于features分裂
			const auto& feats = n->getFeatures();
			// 复制一份用于挑选种子
			std::vector<Feature> allFeats = feats;
			int seedA, seedB;
			pickSeeds(allFeats, seedA, seedB);

			// 创建两个新节点(沿用n本身作为第一个节点)
			RNode* n1 = n;
			RNode* n2 = new RNode(allFeats[seedB].getEnvelope(), maxChildren);

			// 将seedB特征移入n2
			Feature fb = allFeats[seedB];
			n1->remove(fb);
			n2->add(fb);

			// 分配其他特征
			for (size_t i = 0; i < allFeats.size(); ++i) {
				if ((int)i == seedA || (int)i == seedB) continue;
				Feature candidate = allFeats[i];
				Envelope m1 = n1->getEnvelope().unionEnvelope(candidate.getEnvelope());
				Envelope m2 = n2->getEnvelope().unionEnvelope(candidate.getEnvelope());
				double enlarge1 = m1.getArea() - n1->getEnvelope().getArea();
				double enlarge2 = m2.getArea() - n2->getEnvelope().getArea();
				if (enlarge1 < enlarge2) {
					n1->add(candidate);
					n1->setEnvelope(m1);
				}
				else {
					n2->add(candidate);
					n2->setEnvelope(m2);
				}
			}

			// 更新两个节点的包围盒
			Envelope e1 = Envelope::createNull();
			for (auto& ff : n1->getFeatures()) {
				e1 = e1.unionEnvelope(ff.getEnvelope());
			}
			n1->setEnvelope(e1);

			Envelope e2 = Envelope::createNull();
			for (auto& ff : n2->getFeatures()) {
				e2 = e2.unionEnvelope(ff.getEnvelope());
			}
			n2->setEnvelope(e2);

			return { n1, n2 };
		}
		else {
			// 内部节点分裂，基于children分裂
			std::vector<RNode*> nodes;
			for (int i = 0; i < n->getChildNum(); ++i) {
				nodes.push_back(n->getChildNode(i));
			}

			int seedA, seedB;
			pickSeedsForNodes(nodes, seedA, seedB);

			RNode* n1 = n; // 原地使用n作为n1
			RNode* n2 = new RNode(nodes[seedB]->getEnvelope(), maxChildren);

			RNode* nodeB = nodes[seedB];
			n1->remove(nodeB);
			n2->add(nodeB);

			for (size_t i = 0; i < nodes.size(); ++i) {
				if ((int)i == seedA || (int)i == seedB) continue;
				RNode* candidate = nodes[i];
				Envelope m1 = n1->getEnvelope().unionEnvelope(candidate->getEnvelope());
				Envelope m2 = n2->getEnvelope().unionEnvelope(candidate->getEnvelope());
				double enlarge1 = m1.getArea() - n1->getEnvelope().getArea();
				double enlarge2 = m2.getArea() - n2->getEnvelope().getArea();
				if (enlarge1 < enlarge2) {
					// 加入n1
					n1->remove(candidate);
					n1->add(candidate);
					n1->setEnvelope(m1);
				}
				else {
					n1->remove(candidate);
					n2->add(candidate);
					n2->setEnvelope(m2);
				}
			}

			// 更新包围盒
			n1->setEnvelope(computeMBRForChildren(n1));
			n2->setEnvelope(computeMBRForChildren(n2));

			return { n1, n2 };
		}
	}

	Envelope RTree::computeMBRForChildren(RNode* parent) {
		Envelope e = Envelope::createNull();
		if (parent->isLeafNode()) {
			for (auto& f : parent->getFeatures()) {
				e = e.unionEnvelope(f.getEnvelope());
			}
		}
		else {
			for (int i = 0; i < parent->getChildNum(); ++i) {
				e = e.unionEnvelope(parent->getChildNode(i)->getEnvelope());
			}
		}
		return e;
	}

	void RTree::pickSeeds(const std::vector<Feature>& feats, int& seedA, int& seedB) {
		double maxD = -1.0;
		seedA = 0;
		seedB = 1;
		for (size_t i = 0; i < feats.size(); ++i) {
			for (size_t j = i + 1; j < feats.size(); ++j) {
				Envelope m = feats[i].getEnvelope().unionEnvelope(feats[j].getEnvelope());
				double d = m.getArea() - feats[i].getEnvelope().getArea() - feats[j].getEnvelope().getArea();
				if (d > maxD) {
					maxD = d;
					seedA = (int)i;
					seedB = (int)j;
				}
			}
		}
	}

	void RTree::pickSeedsForNodes(const std::vector<RNode*>& nodes, int& seedA, int& seedB) {
		double maxD = -1.0;
		seedA = 0;
		seedB = 1;
		for (size_t i = 0; i < nodes.size(); ++i) {
			for (size_t j = i + 1; j < nodes.size(); ++j) {
				Envelope m = nodes[i]->getEnvelope().unionEnvelope(nodes[j]->getEnvelope());
				double d = m.getArea() - nodes[i]->getEnvelope().getArea() - nodes[j]->getEnvelope().getArea();
				if (d > maxD) {
					maxD = d;
					seedA = (int)i;
					seedB = (int)j;
				}
			}
		}
	}

} // namespace hw6