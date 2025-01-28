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
#include <iomanip>

#include "CMakeIn.h"

namespace hw6 {

	/// <summary>
	/// </summary>
	struct Distance {
		double dist;            // 距离值
		const Feature* feature; // 指向的特征

		Distance(double d, const Feature* f) : dist(d), feature(f) {}

		// 比较运算符，用于最小堆
		bool operator<(const Distance& other) const {
			return dist < other.dist;
		}
	};
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

		void KNNQuery(double x, double y, int k, std::priority_queue<Distance, std::vector<Distance>, std::less<Distance>>& pq);

		void collectAllFeatures(std::vector<Feature>& allFeatures);
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

		//virtual bool spatialJoin(double distance, std::vector<Feature> features, std::vector<std::pair<Feature, Feature>>& result);
		
		//获得当前树中的所有元素
		virtual void getAllFeatures(std::vector<Feature>& features);

		// bool KNNQuery(double x, double y, int k, std::vector<Feature>& features) override;

		// void spatialJoinNestedLoopIndex(std::vector<Feature>& features, double distance, short mode) override;
		
		// void spatialJoinTreeMatching(Tree* tree, double distance, short mode) override;

		bool KNNQuery(double x, double y, int k, std::vector<Feature>& result);
		
		void spatialJoin(double distance, RTree* tree, std::vector<std::pair<Feature, Feature>>& result, int mode);

		RNode* pointInLeafNode(double x, double y) {
			if (root != nullptr) return root->pointInLeafNode(x, y);
			else return nullptr;
		}

		void insertFeature(RNode* node, const Feature& feature);
		void splitNonLeafNode(RNode* node);
		void splitLeafNode(RNode* node);
		void getLeafNodes(std::vector<RNode*>& leafNodes);

		void draw() override { if (root != nullptr) root->draw(); }

		void leafNodesQuery(const Envelope& rect, std::vector<RNode*>& leafNodes);

	public:
		static void test(int t);
		static void analyse();
	};

	

} // namespace hw6

#endif // !RTREE_H_INCLUDED