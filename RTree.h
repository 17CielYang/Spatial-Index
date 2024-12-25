#ifndef RTREE_H_INCLUDED
#define RTREE_H_INCLUDED

#include "Geometry.h"
#include "Tree.h"

#include <algorithm>
#include <array>
#include <queue>
#include <string>
#include <vector>
#include <random>

#include "CMakeIn.h"

namespace hw6 {

	/// <summary>
	/// </summary>
	class RNode {
	private:
		RNode* parent = nullptr; // 初始值为空：根节点没有父节点
		int maxChildren = 8; // 当前节点可以拥有的最大子节点数
		std::vector<RNode*> children; // 存储当前节点的所有子节点的动态数组，数组名为children
		int childrenNum = 0; // 当前节点拥有的子节点数量，初始化为0
		Envelope bbox; // 当前节点的包围盒
		std::vector<Feature> features; // 叶子节点中，feature为实际空间对象

	public:
		RNode() = delete; // 禁用默认构造函数
		RNode(const Envelope& box) : bbox(box) {} // 构造函数：接受一个Envelope对象box，将其赋值给成员变量bbox~RNode();
		~RNode();
		bool isLeafNode() { return childrenNum == 0; }

		const Envelope& getEnvelope() { return bbox; }

		RNode* getParent() { return parent; }

		void setParent(RNode* p) { parent = p; }

		void setEnvelope(const Envelope& box) { bbox = box; }

		RNode* getChildNode(size_t i) { // 返回第i个子节点的指针
			return i < childrenNum ? children[i] : nullptr;
		}

		const RNode* getChildNode(size_t i) const {
			return i < childrenNum ? children[i] : nullptr;
		}

		int getChildNum() const { return children.size(); }

		size_t getFeatureNum() const { return features.size(); }

		const Feature& getFeature(size_t i) const { return features[i]; } // 返回第i个feature

		const std::vector<Feature>& getFeatures() const { return features; } // 返回所有feature的引用

		void add(const Feature& f) { 
			features.push_back(f); 
			bbox = bbox.unionEnvelope(f.getEnvelope());
		}

		void add(RNode* child);

		void remove(const Feature& f);

		void remove(RNode* child);

		Feature popBackFeature(); // 移除并返回最后一个feature

		RNode* popBackChildNode(); // 移除并返回最后一个子节点

		void countNode(int& interiorNum, int& leafNum); // 统计树中的内部节点和叶子节点的数量

		int countHeight(int height); // 记录树的高度（这个参数height的作用是什么？）

		void draw();

		void rangeQuery(const Envelope& rect, std::vector<Feature>& result); // 查找与rect相交的所有feature，存储在result中

		RNode* pointInLeafNode(double x, double y); // 查找包含点(x, y)的叶子节点

		void RNode::recalculateEnvelope() {
			if (isLeafNode()) {
				if (features.empty()) {
					bbox = Envelope(); // 设置为空包围盒
					std::cout << "Node is a leaf and now empty. BBox reset." << std::endl;
				}
				else {
					// 确保 features 中至少有一个要素
					const Envelope& firstEnv = features[0].getEnvelope();
					Envelope env = firstEnv;

					for (size_t i = 1; i < features.size(); ++i) {
						const Envelope& currentEnv = features[i].getEnvelope();
						env = env.unionEnvelope(currentEnv);
					}
					bbox = env;
				}
			}
			else {
				if (children.empty()) { // 使用 children.empty()
					bbox = Envelope(); // 设置为空包围盒
					std::cout << "Node is internal and now has no children. BBox reset." << std::endl;
				}
				else {
					// 确保 children 中至少有一个子节点
					const Envelope& firstEnv = children[0]->getEnvelope();
					Envelope env = firstEnv;

					for (size_t i = 1; i < children.size(); ++i) { // 使用 size_t
						const Envelope& currentEnv = children[i]->getEnvelope();
						env = env.unionEnvelope(currentEnv);
					}
					bbox = env;
				}
			}
		}


		void deleteFeature(const Feature& f) {
			std::cout << "Attempting to remove Feature: " << f.getName() << std::endl;
			auto it = std::find_if(
				features.begin(),
				features.end(),
				[&](const Feature& feature) {
					// 通过名称和几何对象的地址进行匹配
					return (feature.getName() == f.getName()) && (feature.getGeom() == f.getGeom());
				}
			);

			if (it != features.end()) {
				std::cout << "Found Feature: " << it->getName() << ", removing it." << std::endl;
				features.erase(it);
				// recalculateEnvelope();
				if (features.empty()) {
					std::cout << "All features removed from node." << std::endl;
				}
			}
			else {
				std::cerr << "Error: Feature not found: " << f.getName() << std::endl;
			}
		}

		// 删除子节点
		void deleteChild(RNode* child) {
			auto it = std::find(children.begin(), children.end(), child);
			if (it != children.end()) {
				std::cout << "Found child node, removing it." << std::endl;
				children.erase(it);
				child->parent = nullptr;
				// recalculateEnvelope();
			}
			else {
				std::cerr << "Error: Child node not found in node." << std::endl;
			}
		}

	};

	class RTree : public Tree {
	private:
		int maxChildren = 8;
		RNode* root = nullptr; // 树开始时为空

	public:
		// 默认构造函数，maxChildren 默认为 8
		RTree() : Tree(8), maxChildren(8) {}
		explicit RTree(int maxChildren) : Tree(maxChildren), maxChildren(maxChildren) {} // 显式构造函数
		~RTree() override {
			if (root != nullptr) delete root;
			root = nullptr;
		}

		void setCapacity(int capacity) override {
			// DO NOTHING, since capacity is immutable in R tree
		}

		bool constructTree(const std::vector<Feature>& features) override; // 接受一个 Feature 向量，包含所有需要存储在树中的空间特征，构建R树

		void countNode(int& interiorNum, int& leafNum) override;

		void countHeight(int& height) override;

		void rangeQuery(const Envelope& rect, std::vector<Feature>& features) override;

		bool NNQuery(double x, double y, std::vector<Feature>& features) override;

		void draw() override { 
			if (root != nullptr) {
				std::cout << "Drawing RTree." << std::endl;
				root->draw();
			}	 
		}

		void insert(const Feature& f);

		RNode* pointInLeafNode(double x, double y) { // 查找包含点(x,y)的叶子节点
			if (root != nullptr) return root->pointInLeafNode(x, y);
			else return nullptr;
		}

		RNode* chooseLeaf(RNode* current, const Feature& f);

		std::pair<RNode*, RNode*> splitNode(RNode* node);

		void AdjustTree(RNode* node, RNode* newNode);

	public:
		static void test(int t);
		static void analyse();
	};

} // namespace hw6

#endif // !RTREE_H_INCLUDED