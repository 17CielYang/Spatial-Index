﻿#ifndef RTREE_H_INCLUDED
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
		RNode* parent = nullptr;
		int maxChildren;
		std::vector<RNode*> children;
		int childrenNum = 0;
		Envelope bbox;
		std::vector<Feature> features;

	public:
		RNode() = delete;
		RNode(const Envelope& box) : bbox(box) {}

		bool isLeafNode() { return childrenNum == 0; }

		const Envelope& getEnvelope() { return bbox; }

		RNode* getParent() { return parent; }

		void setEnvelope(const Envelope& box) { bbox = box; }

		RNode* getChildNode(size_t i) {
			return i < childrenNum ? children[i] : nullptr;
		}

		const RNode* getChildNode(size_t i) const {
			return i < childrenNum ? children[i] : nullptr;
		}

		int getChildNum() const { return childrenNum; }

		size_t getFeatureNum() const { return features.size(); }

		const Feature& getFeature(size_t i) const { return features[i]; }

		const std::vector<Feature>& getFeatures() const { return features; }

		void add(const Feature& f) { features.push_back(f); }

		void add(RNode* child);

		void remove(const Feature& f);

		void remove(RNode* child);

		Feature popBackFeature();

		RNode* popBackChildNode();

		void countNode(int& interiorNum, int& leafNum);

		int countHeight(int height);

		void draw();

		void rangeQuery(const Envelope& rect, std::vector<Feature>& features);

		void leafNodesQuery(const Envelope& rect, std::vector<RNode*>& leafNodes);

		RNode* pointInLeafNode(double x, double y);
	};

	class RTree : public Tree {
	private:
		int maxChildren;
		RNode* root = nullptr;

	public:
		// 默认构造函数，maxChildren 默认为 8
		RTree() : RTree(8) {}
		explicit RTree(int maxChildren);
		~RTree() {
			if (root != nullptr) delete root;
			root = nullptr;
		}

		void setCapacity(int capacity) override {
			// DO NOTHING, since capacity is immutable in R tree
		}

		bool constructTree(const std::vector<Feature>& features) override;

		void countNode(int& interiorNum, int& leafNum) override;

		void countHeight(int& height) override;

		void rangeQuery(const Envelope& rect, std::vector<Feature>& features) override;

		bool NNQuery(double x, double y, std::vector<Feature>& features) override;

		RNode* pointInLeafNode(double x, double y) {
			if (root != nullptr) return root->pointInLeafNode(x, y);
			else return nullptr;
		}

		void insertFeature(RNode* node, const Feature& feature);
		void splitInnerNode(RNode* node);
		void splitLeafNode(RNode* node);
		void getLeafNodes(std::vector<RNode*>& leafNodes);

		void draw() override { if (root != nullptr) root->draw(); }

		void leafNodesQuery(const Envelope& rect, std::vector<RNode*>& leafNodes);

	public:
		static void test(int t);
		static void analyse();
	};

	struct Distance {
		RNode* pNode;
		double distance;

		// 构造函数
		Distance(RNode* p, double d) : pNode(p), distance(d) {}

		// 排序规则，按距离从小到大排序
		bool operator>(const Distance& other) const {
			return distance > other.distance;  // 越小越排前面
		}
	};

} // namespace hw6

#endif // !RTREE_H_INCLUDED
