#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED
#include <string>
#include<set>
#include "Geometry.h"

namespace hw6 {

	class Feature {
	private:
		//int fid;
		std::string name;
		Geometry* geom;

	public:
		Feature() : geom(NULL) {}
		Feature(const std::string& name, Geometry* geom) : name(name), geom(geom) {}
		//Feature(const std::string& name, Geometry* geom, int fid) : name(name), geom(geom), fid(fid) {}
		const std::string& getName() const { return name; }

		const Geometry* getGeom() const { return geom; }

		const Envelope& getEnvelope() const { return geom->getEnvelope(); }

		//const int getFid() const { return fid; }

		double maxDistance2Envelope(double x, double y) const { // 计算点到Envelope四个角的最大欧式距离
			const Envelope& e = geom->getEnvelope();
			double x1 = e.getMinX();
			double y1 = e.getMinY();
			double x2 = e.getMaxX();
			double y2 = e.getMaxY();

			double d1 = sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));
			double d2 = sqrt((x - x1) * (x - x1) + (y - y2) * (y - y2));
			double d3 = sqrt((x - x2) * (x - x2) + (y - y1) * (y - y1));
			double d4 = sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));

			return std::max(std::max(d1, d2), std::max(d3, d4));
		}
		double minDistance2Envelope(double x, double y) const {
			const Envelope& e = geom->getEnvelope();
			double x1 = e.getMinX();
			double y1 = e.getMinY();
			double x2 = e.getMaxX();
			double y2 = e.getMaxY();

			double d1 = sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));
			double d2 = sqrt((x - x1) * (x - x1) + (y - y2) * (y - y2));
			double d3 = sqrt((x - x2) * (x - x2) + (y - y1) * (y - y1));
			double d4 = sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));
			return std::min(std::min(d1, d2), std::min(d3, d4));

		}
		
		double distance(double x, double y) const {
			Point point(x, y);
			return geom->distance(&point);
		}

		void print() const {
			std::cout << "Feature: " << name << " ";
			geom->print();
		}

		void draw() const {
			if (geom)
				geom->draw();
		}
	};

	class Tree {
	protected:
		size_t capacity;
		Envelope bbox;

	public:
		Tree(size_t cap) : capacity(cap) {}
		virtual ~Tree() {}

		int getCapacity() const { return capacity; }

		virtual void setCapacity(int capacity) { this->capacity = capacity; }

		const Envelope& getEnvelope() const { return bbox; }

		virtual bool constructTree(const std::vector<Feature>& features) = 0;

		virtual void countNode(int& interiorNum, int& leafNum) = 0;

		virtual void countHeight(int& height) = 0;

		virtual void rangeQuery(const Envelope& rect, std::vector<Feature>& features) = 0;

		virtual bool NNQuery(double x, double y, std::vector<Feature>& features) = 0;
		virtual bool KNNQuery(double x, double y, int k,std::vector<Feature>& features) = 0;

		//virtual bool spatialJoin(double distance, std::vector<Feature> features, std::vector<std::pair<Feature, Feature>>& result) = 0;
		virtual void getAllFeatures(std::vector<Feature>& features)=0;

		virtual void draw() = 0;
	};

} // namespace hw6

#endif // !TREE_H_INCLUDED