#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include <algorithm>
#include <iostream>
#include <vector>

namespace hw6 {

	class Point;
	class LineString;
	class Polygon;
	class MultiPoint;
	class MultiLineString;
	class MultiPolygon;



	class Envelope {
	private:
		double minX;
		double minY;
		double maxX;
		double maxY;

	public:
		Envelope() : minX(0), minY(0), maxX(0), maxY(0) {}
		Envelope(double minX, double maxX, double minY, double maxY)
			: minX(minX), minY(minY), maxX(maxX), maxY(maxY) {}

		double getMinX() const { return minX; }
		double getMinY() const { return minY; }
		double getMaxX() const { return maxX; }
		double getMaxY() const { return maxY; }

		double getWidth() const { return maxX - minX; }
		double getHeight() const { return maxY - minY; }
		double getArea() const { return getWidth() * getHeight(); }

		bool contain(double x, double y) const;

		double minDistance(double x, double y) const;
		double minDistance(const Envelope& other) const;

		void draw() const;

		void print() const {
			std::cout << "Envelope( " << minX << " " << maxX << " " << minY << " "
				<< maxY << ") ";
		}

		bool operator==(const Envelope& t1) const {
			return (minX == t1.minX && minY == t1.minY && maxX == t1.maxX &&
				maxY == t1.maxY);
		}
		bool operator!=(const Envelope& t1) const { return !(*this == t1); }

		bool contain(const Envelope& envelope) const;
		bool intersect(const Envelope& envelope) const;
		Envelope unionEnvelope(const Envelope& envelope) const;
		Envelope expand(double d) const {
			return Envelope(minX - d, minY - d, maxX + d, maxY + d);
		}
	};
	
	/*
	 * Geometry hierarchy
	 */
	class Geometry {
	protected:
		Envelope envelope;

	public:
		Geometry() {}
		virtual ~Geometry() {}

		const Envelope& getEnvelope() const { return envelope; }

		virtual void constructEnvelope() = 0;
		virtual double distance(const Geometry* geom) const {
			return geom->distance(this);
		} // Euclidean distance
		virtual double distance(const Point* point) const = 0;
		virtual double distance(const LineString* line) const = 0;
		virtual double distance(const Polygon* polygon) const = 0;
		virtual bool intersects(const Envelope& rect) const = 0;
		virtual void draw() const = 0;
		virtual void print() const = 0;
	};

	class Point : public Geometry {
	private:
		double x;
		double y;

	public:
		Point() : x(0), y(0) {}
		Point(double x, double y) : x(x), y(y) { constructEnvelope(); }
		virtual ~Point() {}

		double getX() const { return x; }
		double getY() const { return y; }

		virtual void constructEnvelope() { envelope = Envelope(x, x, y, y); }

		// Euclidean distance
		virtual double distance(const Point* point) const;
		virtual double distance(const LineString* line) const;
		virtual double distance(const Polygon* polygon) const;

		// intersection test with the envelope for range query
		virtual bool intersects(const Envelope& rect) const;

		virtual void draw() const;

		virtual void print() const {
			std::cout << "Point(" << x << " " << y << ")";
		}
	};

	class LineString : public Geometry {
	private:
		std::vector<Point> points;

	public:
		LineString() {}
		LineString(std::vector<Point>& pts) : points(pts) { constructEnvelope(); }
		virtual ~LineString() {}

		size_t numPoints() const { return points.size(); }
		Point getStartPoint() const { return points.front(); }
		Point getEndPoint() const { return points.back(); }
		Point getPointN(size_t n) const { return points[n]; }

		virtual void constructEnvelope();

		// Euclidean distance
		virtual double distance(const Point* point) const {
			return point->distance(this);
		}
		virtual double distance(const LineString* line) const;
		virtual double distance(const Polygon* polygon) const;

		// intersection test with the envelope for range query
		virtual bool intersects(const Envelope& rect) const;
		virtual bool intersects(const LineString* line) const;

		virtual bool contain(const Point& p) const;
		virtual bool contain(const LineString& line) const;

		virtual void draw() const;

		virtual void print() const;
	};

	class Polygon : public Geometry {
	private:
		LineString exteriorRing;
		size_t innerRingsNum;
		std::vector<LineString> innerRings;

	public:
		Polygon() {}
		Polygon(LineString& ering) : exteriorRing(ering),innerRingsNum(0) { constructEnvelope(); }
		Polygon(const LineString& exteriorRings, const std::vector<LineString>& innerRings)
			: exteriorRing(exteriorRings), innerRings(innerRings), innerRingsNum(innerRings.size()) {
			constructEnvelope();
		}
		virtual ~Polygon() {}

		LineString getExteriorRing() const { return exteriorRing; }
		LineString getInnerRingN(int i)const { return innerRings[i]; }
		size_t getInnerRingNum() const { return innerRingsNum; }

		virtual void constructEnvelope() { envelope = exteriorRing.getEnvelope(); }

		// Euclidean distance
		virtual double distance(const Point* point) const {
			return point->distance(this);
		}
		virtual double distance(const LineString* line) const {
			return line->distance(this);
		}
		virtual double distance(const Polygon* polygon) const;

		// intersection test with the envelope for range query
		virtual bool intersects(const Envelope& rect) const;

		virtual void draw() const;

		virtual void print() const;
	};

	// MultiPoint 类
	class MultiPoint : public Geometry {
	public:
		std::vector<Point> points;

		MultiPoint(const std::vector<Point>& points) : points(points) {}

		// 添加一个点
		void addPoint(const Point& point) {
			points.push_back(point);
		}

		// 获取点的数量
		size_t size() const {
			return points.size();
		}

		virtual void draw() {
			for (const auto& point : points) {
				point.draw();  // 绘制每个点
			}
		}
	};
	// MultiLineString 类
	class MultiLineString : public Geometry {
	public:
		std::vector<LineString> lines;

		MultiLineString(const std::vector<LineString>& lines) : lines(lines) {}

		// 添加一条线段
		void addLine(const LineString& line) {
			lines.push_back(line);
		}

		// 获取线段的数量
		size_t size() const {
			return lines.size();
		}

		// 绘制多个线段
		void draw() const override {
			for (const auto& line : lines) {
				line.draw();  // 绘制每条线段
			}
		}
	};

	// MultiPolygon 类
	class MultiPolygon : public Geometry {
	public:
		std::vector<Polygon> polygons;

		MultiPolygon(const std::vector<Polygon>& polygons) : polygons(polygons) {}

		// 添加一个多边形
		void addPolygon(const Polygon& polygon) {
			polygons.push_back(polygon);
		}

		// 获取多边形的数量
		size_t size() const {
			return polygons.size();
		}

		// 绘制多个多边形
		void draw() const override {
			for (const auto& polygon : polygons) {
				polygon.draw();  // 绘制每个多边形
			}
		}
	};

} // namespace hw6

#endif