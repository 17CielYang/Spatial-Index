#include "Geometry.h"
#include <cmath>
#include <gl/freeglut.h>
#include<algorithm>

#define NOT_IMPLEMENT -1.0

namespace hw6 {

	/*
	 * Envelope functions
	 */
	bool Envelope::contain(double x, double y) const {
		return x >= minX && x <= maxX && y >= minY && y <= maxY;
	}

	bool Envelope::contain(const Envelope& envelope) const {
		// Task 测试Envelope是否包含关系
		// TODO
		if (envelope.getMaxX()<= maxX && envelope.getMaxY() <= maxY && envelope.getMinX()>= minX && envelope.getMinY()>= minY)
			return true;
		return false;
	}

	bool Envelope::intersect(const Envelope& envelope) const {
		// Task 测试Envelope是否相交
		// TODO
		if(envelope.getMaxX()<minX|| envelope.getMinX()>maxX|| envelope.getMaxY()<minY|| envelope.getMinY()>maxY)
			return false;
		return true;
	}

	Envelope Envelope::unionEnvelope(const Envelope& envelope) const {
		if (envelope.contain(*this)) {
			return envelope; // 如果是，返回包含另一个Envelope的Envelope
		}
		else if (this->contain(envelope)) {
			return *this; // 如果是，返回包含传入Envelope的Envelope
		}
		else {
			// 合并两个Envelope
			double minX = std::min(this->minX, envelope.minX);
			double maxX = std::max(this->maxX, envelope.maxX);
			double minY = std::min(this->minY, envelope.minY);
			double maxY = std::max(this->maxY, envelope.maxY);
			return Envelope(minX, maxX, minY, maxY);
		}
	}

	double Envelope::minDistance(double x, double y) const {
		double dx = 0.0;
		if (x < minX) dx = minX - x;
		else if (x > maxX) dx = x - maxX;

		double dy = 0.0;
		if (y < minY) dy = minY - y;
		else if (y > maxY) dy = y - maxY;

		if (dx == 0.0 && dy == 0.0) return 0.0;
		return std::sqrt(dx * dx + dy * dy);
	}

	double Envelope::minDistance(const Envelope& other) const {
		if (this->intersect(other)) {
			return 0.0;
		}

		double dx = 0.0;
		// 如果 other 完全在 this 的左侧
		if (other.getMaxX() < this->getMinX()) {
			dx = this->getMinX() - other.getMaxX();
		}
		// 如果 other 完全在 this 的右侧
		else if (other.getMinX() > this->getMaxX()) {
			dx = other.getMinX() - this->getMaxX();
		}

		double dy = 0.0;
		// 如果 other 完全在 this 的下方
		if (other.getMaxY() < this->getMinY()) {
			dy = this->getMinY() - other.getMaxY();
		}
		// 如果 other 完全在 this 的上方
		else if (other.getMinY() > this->getMaxY()) {
			dy = other.getMinY() - this->getMaxY();
		}


		return std::sqrt(dx * dx + dy * dy);
	}

	void Envelope::draw() const {
		glBegin(GL_LINE_LOOP);

		glVertex2d(minX, minY);
		glVertex2d(minX, maxY);
		glVertex2d(maxX, maxY);
		glVertex2d(maxX, minY);
		glVertex2d(minX, minY);

		glEnd();
	}

	/*
	 * Points functions
	 */
	double Point::distance(const Point* point) const {
		return sqrt((x - point->x) * (x - point->x) +
			(y - point->y) * (y - point->y));
	}

	double Point::distance(const LineString* line) const {
		double mindist = line->getPointN(0).distance(this);
		for (size_t i = 0; i < line->numPoints() - 1; ++i) {
			double dist = 0;
			double x1 = line->getPointN(i).getX();
			double y1 = line->getPointN(i).getY();
			double x2 = line->getPointN(i + 1).getX();
			double y2 = line->getPointN(i + 1).getY();
			// Task calculate the distance between Point P(x, y) and Line [P1(x1,
			// y1), P2(x2, y2)] (less than 10 lines)
			// TODO
			double vx = (x2 - x1);
			double vy = (y2 - y1);
			double vx1 = (x - x1);
			double vy1 = (y - y1);
			double vx2 = (x - x2);
			double vy2 = (y - y2);
			if ((vx * vx1 + vy * vy1) < 0){		//如果P(x,y)与线段成钝角
				dist = line->getPointN(i).distance(this);
			}
			else if ((vx * vx2 + vy * vy2) > 0) {
				dist = line->getPointN(i+1).distance(this);
			}
			else {
				dist = fabs((y1 - y2) * x - (x1 - x2) * y + y2 * x1 - y1 * x2) / sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
			}
			if (dist < mindist)
				mindist = dist;
		}
		return mindist;
	}

	double Point::distance(const Polygon* polygon) const {
		LineString line = polygon->getExteriorRing();

		bool inPolygon = line.contain(*this);
		int inRing = -1;
		// Task whether Point P(x, y) is within Polygon (less than 15 lines)
		// TODO
		if (polygon->getInnerRingNum() != 0 && inPolygon) 
		//如果point在内环当中，不在polygon的范围内，计算point离所处内环的距离
			for (size_t i = 0; i < polygon->getInnerRingNum(); i++) {
				if (polygon->getInnerRingN(i).contain(*this)) {
					inRing = i;
					inPolygon = false;
					break;
				}
			}
		
		double mindist = 0;
		if (!inPolygon ) 
			if (inRing == -1) {
				mindist = this->distance(&line);
			}
			else {
				mindist = this->distance(&polygon->getInnerRingN(inRing));
			}
		return mindist;
	}

	bool Point::intersects(const Envelope& rect) const {
		return (x >= rect.getMinX()) && (x <= rect.getMaxX()) &&
			(y >= rect.getMinY()) && (y <= rect.getMaxY());
	}

	void Point::draw() const {
		glBegin(GL_POINTS);
		glVertex2d(x, y);
		glEnd();
	}

	/*
	 * LineString functions
	 */
	void LineString::constructEnvelope() {
		double minX, minY, maxX, maxY;
		maxX = minX = points[0].getX();
		maxY = minY = points[0].getY();
		for (size_t i = 1; i < points.size(); ++i) {
			maxX = std::max(maxX, points[i].getX());
			maxY = std::max(maxY, points[i].getY());
			minX = std::min(minX, points[i].getX());
			minY = std::min(minY, points[i].getY());
		}
		envelope = Envelope(minX, maxX, minY, maxY);
	}

	double cross(double x1, double y1, double x2, double y2, double x3, double y3) {
		return (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
	}
	bool on_segment(double x1, double y1, double x2, double y2, double x3, double y3) {
		return (std::min(x1, x2) <= x3 && x3 <= std::max(x1, x2)) &&
			(std::min(y1, y2) <= y3 && y3 <= std::max(y1, y2));
	}
	bool LineString::intersects(const LineString* line) const {
		for (size_t i = 0; i < this->numPoints() - 1; i++) {
			double y1 = this->getPointN(i).getY();
			double y2 = this->getPointN(i + 1).getY();
			double x1 = this->getPointN(i).getX();
			double x2 = this->getPointN(i + 1).getX();
			for (size_t j = 0; j < line->numPoints() - 1; j++) {
				double y3 = line->getPointN(j).getY();
				double y4 = line->getPointN(j + 1).getY();
				double x3 = line->getPointN(j).getX();
				double x4 = line->getPointN(j + 1).getX();

				double cross1 = cross(x1, y1, x2, y2, x3, y3);
				double cross2 = cross(x1, y1, x2, y2, x4, y4);
				double cross3 = cross(x3, y3, x4, y4, x1, y1);
				double cross4 = cross(x3, y3, x4, y4, x2, y2);

				if (cross1 * cross2 <= 0 && cross3 * cross4 <= 0) { return true; }
				if (cross1 == 0 && on_segment(x1, y1, x2, y2, x3, y3))
					return true;
				if (cross2 == 0 && on_segment(x1, y1, x4, y4, x2, y2))
					return true;
				if (cross3 == 0 && on_segment(x3, y3, x1, y1, x4, y4))
					return true;
				if (cross4 == 0 && on_segment(x3, y3, x2, y2, x4, y4))
					return true;
			}
		}
		return false;
	}

	double LineString::distance(const LineString* line) const {
		// TODO
		double mindist = this->getPointN(0).distance(line);
		double dist1 = 0;
		double dist2 = 0;
		if (this->intersects(line))
			return 0;
		for (size_t i = 0; i < this->numPoints() - 1; i++) {
			for (size_t j = 0; j < line->numPoints() - 1; j++) {
				dist1 = this->getPointN(i).distance(line);
				dist2 = line->getPointN(j).distance(this);
				if (dist1 < dist2 && dist1 < mindist)
					mindist = dist1;
				if (dist2 <= dist1 && dist2 < mindist)
					mindist = dist2;
			}
		}
		return mindist;
	}

	double LineString::distance(const Polygon* polygon) const {
		// TODO
		LineString outline = polygon->getExteriorRing();
		if (this->intersects(&outline))
			return 0;
		else if(outline.contain(*this)){
			if (polygon->getInnerRingNum() == 0) {
				return 0;
			}
			else {
				bool innerIntersect = false;
				for (size_t i = 0; i < polygon->getInnerRingNum(); i++) {
					if (polygon->getInnerRingN(i).contain(*this)) {
						std::cout << "contain" << std::endl;
						return polygon->getInnerRingN(i).distance(this);
					}
					if (polygon->getInnerRingN(i).intersects(this))
					{
						innerIntersect = true;
						return 0;
					}
				}
				if (!innerIntersect) return 0;
			}
		}
		else {
			return outline.distance(this);
		}
		return NOT_IMPLEMENT;
	}

	bool LineString::contain(const Point& p) const {
		if (this->getStartPoint().getX() != this->getEndPoint().getX())	//如果起始点和终点不一样，那么不构成polygon
			return false;
		double x = p.getX();
		double y = p.getY();
		int intersect = 0;
		for (size_t i = 0; i < this->numPoints() - 1; i++) {
			double y1 = this->getPointN(i).getY();
			double y2 = this->getPointN(i + 1).getY();
			double x1 = this->getPointN(i).getX();
			double x2 = this->getPointN(i + 1).getX();
			if (fabs(y - y2) + fabs(y - y1) == fabs(y1 - y2)) {
				if (y == y1) {
					if (y2 <= y && x < x1) intersect++;
				}
				if (y == y2) {
					if (y1 <= y && x < x2) intersect++;
				}
				if (y1 != y && y2 != y) {
					if (y1 == y2)
						continue;
					double intersect_x = (x1 * (y - y2) - x2 * (y - y1)) / (y1 - y2);
					if (intersect_x > x) intersect++;
				}
			}
		}
			if (intersect % 2 == 1)
				return true;
			else
				return false;
		}

	bool LineString::contain(const LineString& line) const {
		if (this->getStartPoint().getX() != this->getEndPoint().getX())	//如果起始点和终点不一样，那么不构成polygon
			return false;
		//判断某个LineString是否全部在某个单圈polygon当中
		std::vector<Point> pts;
		for (size_t i = 0; i < line.numPoints()-1; i++) {
			Point a = line.getPointN(i);
			Point b = line.getPointN(i + 1);
			if (!this->contain(a))
				return false;

			pts.push_back(a);
			pts.push_back(b);
			LineString l = LineString(pts);

			if (this->intersects(&l))
				return false;
			pts.clear();
		}
		return true;
	}
		
	
	typedef int OutCode;

		const int INSIDE = 0; // 0000
		const int LEFT = 1;   // 0001
		const int RIGHT = 2;  // 0010
		const int BOTTOM = 4; // 0100
		const int TOP = 8;    // 1000

		// Compute the bit code for a point (x, y) using the clip rectangle
		// bounded diagonally by (xmin, ymin), and (xmax, ymax)
		// ASSUME THAT xmax, xmin, ymax and ymin are global constants.
		OutCode ComputeOutCode(double x, double y, double xmin, double xmax,
			double ymin, double ymax) {
			OutCode code;

			code = INSIDE; // initialised as being inside of [[clip window]]

			if (x < xmin) // to the left of clip window
				code |= LEFT;
			else if (x > xmax) // to the right of clip window
				code |= RIGHT;
			if (y < ymin) // below the clip window
				code |= BOTTOM;
			else if (y > ymax) // above the clip window
				code |= TOP;

			return code;
		}

		// Cohen–Sutherland clipping algorithm clips a line from
		// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with
		// diagonal from (xmin, ymin) to (xmax, ymax).
		bool intersectTest(double x0, double y0, double x1, double y1, double xmin,
			double xmax, double ymin, double ymax) {
			// compute outcodes for P0, P1, and whatever point lies outside the clip
			// rectangle
			OutCode outcode0 = ComputeOutCode(x0, y0, xmin, xmax, ymin, ymax);
			OutCode outcode1 = ComputeOutCode(x1, y1, xmin, xmax, ymin, ymax);
			bool accept = false;

			while (true) {
				
				if (!(outcode0 | outcode1)) {
					// bitwise OR is 0: both points inside window; trivially accept and
					// exit loop
					accept = true;
					break;
				}
				else if (outcode0 & outcode1) {
					// bitwise AND is not 0: both points share an outside zone (LEFT,
					// RIGHT, TOP, or BOTTOM), so both must be outside window; exit loop
					// (accept is false)
					break;
				}
				else {
					// failed both tests, so calculate the line segment to clip
					// from an outside point to an intersection with clip edge
					double x, y;

					// At least one endpoint is outside the clip rectangle; pick it.
					OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

					// Now find the intersection point;
					// use formulas:
					//   slope = (y1 - y0) / (x1 - x0)
					//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
					//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
					// No need to worry about divide-by-zero because, in each case, the
					// outcode bit being tested guarantees the denominator is non-zero
					if (outcodeOut & TOP) { // point is above the clip window
						x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
						y = ymax;
					}
					else if (outcodeOut & BOTTOM) { // point is below the clip window
						x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
						y = ymin;
					}
					else if (outcodeOut &
						RIGHT) { // point is to the right of clip window
						y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
						x = xmax;
					}
					else if (outcodeOut &
						LEFT) { // point is to the left of clip window
						y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
						x = xmin;
					}

					// Now we move outside point to intersection point to clip
					// and get ready for next pass.
					if (outcodeOut == outcode0) {
						x0 = x;
						y0 = y;
						outcode0 = ComputeOutCode(x0, y0, xmin, xmax, ymin, ymax);
					}
					else {
						x1 = x;
						y1 = y;
						outcode1 = ComputeOutCode(x1, y1, xmin, xmax, ymin, ymax);
					}
				}
			}
			return accept;
		
	}

	bool LineString::intersects(const Envelope& rect) const {
		double xmin = rect.getMinX();
		double xmax = rect.getMaxX();
		double ymin = rect.getMinY();
		double ymax = rect.getMaxY();

		for (size_t i = 1; i < points.size(); ++i)
			if (intersectTest(points[i - 1].getX(), points[i - 1].getY(),
				points[i].getX(), points[i].getY(), xmin, xmax, ymin,
				ymax))
				return true;
		return false;
	}

	void LineString::draw() const {
		if (points.empty()) {
			std::cerr << "Warning: LineString points array is empty." << std::endl;
			return;
		}

		glBegin(GL_LINE_STRIP);
		for (size_t i = 0; i < points.size(); ++i)
			glVertex2d(points[i].getX(), points[i].getY());
		glEnd();

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL Error in draw(): " << gluErrorString(err) << std::endl;
		}
	}

	void LineString::print() const {
		std::cout << "LineString(";
		for (size_t i = 0; i < points.size(); ++i) {
			if (i != 0)
				std::cout << ", ";
			std::cout << points[i].getX() << " " << points[i].getY();
		}
		std::cout << ")";
	}

	/*
	 * Polygon
	 */
	double Polygon::distance(const Polygon* polygon) const {
		return std::min(exteriorRing.distance(polygon),
			polygon->getExteriorRing().distance(this));
	}

	bool Polygon::intersects(const Envelope& rect) const {
		// TODO
		Point p[4];
		p[0] = Point(rect.getMinX(), rect.getMinY());
		p[1] = Point(rect.getMaxX(), rect.getMinY());
		p[2] = Point(rect.getMaxX(), rect.getMaxY());
		p[3] = Point(rect.getMinX(), rect.getMaxY());

		for (int i = 0; i < 4; i++) {
			if (p[i].distance(this) == 0) {
				return true;
			}
		}
			LineString line = exteriorRing;
			for (size_t i = 0; i < line.numPoints(); i++) {
				if (rect.contain(line.getPointN(i).getX(), line.getPointN(i).getY()) )
					return true;
			}
			if (innerRingsNum != 0) {
				for (size_t i = 0; i < innerRingsNum; i++) {
					line = innerRings[i];
					for (size_t j = 0; j < line.numPoints(); j++) {
						if (rect.contain(line.getPointN(j).getX(), line.getPointN(j).getY()))
							return true;
					}
				}
			}

		return false;
	}

	void Polygon::draw() const 
	{ 
		exteriorRing.draw();
		if (this->getInnerRingNum() != 0) {
			for (auto& ring : innerRings) {
				ring.draw();
			}
		}
	}

	void Polygon::print() const {
		std::cout << "Polygon(";
		if(innerRingsNum==0)
		for (size_t i = 0; i < exteriorRing.numPoints(); ++i) {
			if (i != 0)
				std::cout << ", ";
			Point p = exteriorRing.getPointN(i);
			std::cout << p.getX() << " " << p.getY();
		}
		else {
			std::cout << "(";
			for (size_t i = 0; i < exteriorRing.numPoints(); ++i) {
				if (i != 0)
					std::cout << ", ";
				Point p = exteriorRing.getPointN(i);
				std::cout << p.getX() << " " << p.getY();
			}
			std::cout << "),(";
			for (size_t i=0; i < innerRingsNum; i++) {
				LineString ring = innerRings[i];
				for (size_t j = 0; j < ring.numPoints(); ++j) {
					if (j != 0)
						std::cout << ", ";
					Point p = ring.getPointN(j);
					std::cout << p.getX() << " " << p.getY();
				}
				if (i != innerRingsNum - 1)
					std::cout << "),(";
			}
			std::cout << ")";

		}
		std::cout << ")";
	}

} // namespace hw6