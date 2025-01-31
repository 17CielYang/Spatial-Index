﻿
#include "QuadTree.h"
#include "Common.h"
#include <ctime>
#include "CMakeIn.h"

using namespace hw6;

extern int mode;
extern std::vector<Geometry*> readGeom(const char* filename);
extern std::vector<std::string> readName(const char* filename);
extern void transformValue(double& res, const char* format);
extern void wrongMessage(Envelope e1, Envelope e2, bool cal);
extern void wrongMessage(const Point& pt1, const Point& pt2, double dis,
	double res);
extern void wrongMessage(Envelope e1, Envelope e2, Envelope cal, Envelope res);

namespace hw6 {

	void QuadTree::analyse() {
		using namespace std;

		vector<Feature> features;
		vector<Geometry*> geom = readGeom(PROJ_SRC_DIR "/data/taxi");
		vector<string> name = readName(PROJ_SRC_DIR "/data/taxi");

		features.clear();
		features.reserve(geom.size());
		for (size_t i = 0; i < geom.size(); ++i)
			features.push_back(Feature(name[i], geom[i]));

		cout << "taxi number: " << geom.size() << endl;

		srand(time(nullptr));
		for (int cap = 70; cap <= 200; cap += 10) {
			// Task 构造四叉树，输出四叉树的节点数目和高度
			// TODO
			QuadTree* qtree = new QuadTree(cap);

			clock_t start_time = clock();
			// TODO
			qtree->constructTree(features);
			clock_t end_time = clock();
			int height = 0, interiorNum = 0, leafNum = 0;
			// TODO
			qtree->countHeight(height);
			qtree->countNode(interiorNum,leafNum);

			cout << "Capacity " << cap << "\n";
			cout << "Height: " << height
				<< " \tInterior node number: " << interiorNum
				<< " \tLeaf node number: " << leafNum << "\n";
			cout << "Construction time: " << (end_time - start_time) / 1000.0 << "s"
				<< endl;

			double x, y;
			vector<Feature> candidateFeatures;
			start_time = clock();
			//随机生成一个点进行点查询
			const Envelope& envelope = qtree->root->getEnvelope();
			double minDist = std::max(envelope.getWidth(), envelope.getHeight());
			Feature NearestFeature;
			for (int i = 0; i < 100; ++i) {
				candidateFeatures.clear();
				x = -((rand() % 225) / 10000.0 + 73.9812);
				y = (rand() % 239) / 10000.0 + 40.7247;
				qtree->NNQuery(x, y, candidateFeatures);
				 cout << "第" << i << "次测试,candidateFeatures有" << candidateFeatures.size() << endl;
				// refine step
				// TODO
				for (Feature& feature : candidateFeatures) {
					if (minDist > feature.distance(x, y)) {
						minDist = feature.distance(x, y);
						NearestFeature = feature;
					}
				}
			}
			end_time = clock();
			cout << "NNQuery time: " << (end_time - start_time) / 1000.0 << "s"
				<< endl
				<< endl;

			delete qtree;
		}
	}

	void QuadTree::test(int t) {
		using namespace std;

		cout << "*********************Start*********************" << endl;
		if (t == TEST1) {
			cout << "测试1: Envelope Contain, Intersect, and Union" << endl;

			int failedCase = 0;
			Envelope e1(-1, 1, -1, 1);
			vector<Envelope> tests;
			tests.push_back(Envelope(-0.5, 0.5, -0.5, 0.5));
			tests.push_back(Envelope(-0.5, 0.5, 0.5, 1.5));
			tests.push_back(Envelope(0.5, 1.5, -0.5, 0.5));
			tests.push_back(Envelope(-1.5, -0.5, -1.5, -0.5));
			tests.push_back(Envelope(-2, -1, -0.5, 0.5));
			tests.push_back(Envelope(1, 1.5, 1, 1.5));
			tests.push_back(Envelope(-2, -1.5, -0.5, 0.5));
			tests.push_back(Envelope(-0.5, 0.5, 1.5, 2));
			tests.push_back(Envelope(-2, -1.5, 0.5, 1.5));
			tests.push_back(Envelope(0.5, 1.5, 1.5, 2));

			for (size_t i = 0; i < tests.size(); ++i) {
				if (e1.contain(tests[i]) != (i == 0)) {
					failedCase += 1;
					wrongMessage(e1, tests[i], (i != 0));
				}
				if (tests[i].contain(e1) == true) {
					failedCase += 1;
					wrongMessage(tests[i], e1, true);
				}
			}
			cout << "Envelope Contain: " << tests.size() * 2 - failedCase << " / "
				<< tests.size() * 2 << " tests are passed" << endl;

			failedCase = 0;
			for (size_t i = 0; i < tests.size(); ++i) {
				if (e1.intersect(tests[i]) != (i < 6)) {
					failedCase += 1;
					wrongMessage(e1, tests[i], (i < 6));
				}
				if (tests[i].intersect(e1) != (i < 6)) {
					failedCase += 1;
					wrongMessage(tests[i], e1, (i < 6));
				}
			}
			cout << "Envelope Intersect: " << tests.size() * 2 - failedCase << " / "
				<< tests.size() * 2 << " tests are passed" << endl;

			failedCase = 0;
			vector<Envelope> results;
			results.push_back(Envelope(-1, 1, -1, 1));
			results.push_back(Envelope(-1, 1, -1, 1.5));
			results.push_back(Envelope(-1, 1.5, -1, 1));
			results.push_back(Envelope(-1.5, 1, -1.5, 1));
			results.push_back(Envelope(-2, 1, -1, 1));
			results.push_back(Envelope(-1, 1.5, -1, 1.5));
			results.push_back(Envelope(-2, 1, -1, 1));
			results.push_back(Envelope(-1, 1, -1, 2));
			results.push_back(Envelope(-2, 1, -1, 1.5));
			results.push_back(Envelope(-1, 1.5, -1, 2));
			for (size_t i = 0; i < tests.size(); ++i) {
				if (e1.unionEnvelope(tests[i]) != results[i]) {
					failedCase += 1;
					wrongMessage(e1, tests[i], e1.unionEnvelope(tests[i]),
						results[i]);
				}
				if (tests[i].unionEnvelope(e1) != results[i]) {
					failedCase += 1;
					wrongMessage(tests[i], e1, e1.unionEnvelope(tests[i]),
						results[i]);
				}
			}
			cout << "Envelope Union: " << tests.size() * 2 - failedCase << " / "
				<< tests.size() * 2 << " tests are passed" << endl;
		}
		else if (t == TEST2) {
			cout << "测试2: Distance between Point and LineString" << endl;

			vector<Point> points;
			points.push_back(Point(0, 0));
			points.push_back(Point(10, 10));
			LineString line(points);

			points.push_back(Point(-10, -10));
			points.push_back(Point(20, 20));
			points.push_back(Point(5, 5));
			points.push_back(Point(10, 0));
			points.push_back(Point(10, -10));
			points.push_back(Point(0, 10));
			points.push_back(Point(0, 20));
			points.push_back(Point(20, 0));

			double dists[] = { 0,       0,       14.1421, 14.1421, 0,
							  7.07107, 14.1421, 7.07107, 14.1421, 14.1421 };

			int failedCase = 0;
			for (size_t i = 0; i < points.size(); ++i) {
				double dist = points[i].distance(&line);
				if (fabs(dist - dists[i]) > 0.0001) {
					failedCase += 1;
					cout << "Your answer is " << dist << " for test between ";
					line.print();
					cout << " and ";
					points[i].print();
					cout << ", but the answer is " << dists[i] << endl;
				}
			}
			cout << "Distance between Point and LineString: "
				<< points.size() - failedCase << " / " << points.size()
				<< " tests are passed" << endl;
		}
		else if (t == TEST3) {
			cout << "测试3: Distance between Point and Polygon" << endl;

			vector<Point> points;
			points.push_back(Point(5, 0));
			points.push_back(Point(3, 6));
			points.push_back(Point(2, 4));
			points.push_back(Point(-2, 4));
			points.push_back(Point(-3, 5));
			points.push_back(Point(-5, 0));
			points.push_back(Point(0, -3));
			points.push_back(Point(5, 0));
			LineString line(points);
			Polygon poly(line);

			points.clear();
			points.push_back(Point(5, 4));
			points.push_back(Point(3, 4));
			points.push_back(Point(0, 4));
			points.push_back(Point(-3, 4));
			points.push_back(Point(-5, 4));
			points.push_back(Point(5, 5));
			points.push_back(Point(3, 5));
			points.push_back(Point(0, 5));
			points.push_back(Point(-3, 5));
			points.push_back(Point(0, 0));

			double dists[] = { 1.26491, 0, 0, 0, 1.48556, 1.58114, 0, 1, 0, 0 };

			int failedCase = 0;
			for (size_t i = 0; i < points.size(); ++i) {
				double dist = points[i].distance(&poly);
				if (fabs(dist - dists[i]) > 0.00001) {
					failedCase += 1;
					cout << "Your answer is " << dist << " for test between ";
					poly.print();
					cout << " and ";
					points[i].print();
					cout << ", but the answer is " << dists[i] << endl;
				}
			}
			cout << "Distance between Point and Polygon: "
				<< points.size() - failedCase << " / " << points.size()
				<< " tests are passed" << endl;
		}
		else if (t == TEST4) {
			cout << "测试4: QuadTree Construction" << endl;
			int ncase, cct;
			ncase = cct = 2;
			QuadTree qtree;
			vector<Geometry*> geom = readGeom(PROJ_SRC_DIR "/data/polygon");
			vector<Feature> features;

			for (size_t i = 0; i < geom.size(); ++i)
				features.push_back(Feature("", geom[i]));

			qtree.setCapacity(1);
			qtree.constructTree(features);

			int height, interiorNum, leafNum;
			qtree.countHeight(height);
			qtree.countNode(interiorNum, leafNum);

			if (!(height == 6 && interiorNum == 8 && leafNum == 25)) {
				cout << "Case 1: "
					<< "Your answer is height: " << height
					<< ", interiorNum: " << interiorNum << ", leafNum: " << leafNum
					<< " for case1, but the answer is height: 6, interiorNum: 8, "
					"leafNum: 25\n";
				--cct;
			}

			features.clear();
			for (size_t i = 0; i < geom.size(); ++i)
				delete geom[i];
			geom.clear();

			vector<Geometry*> geom2 = readGeom(PROJ_SRC_DIR "/data/highway");
			vector<Feature> features2;
			QuadTree qtree2;

			for (size_t i = 0; i < geom2.size(); ++i)
				features2.push_back(Feature("", geom2[i]));

			qtree2.setCapacity(20);
			qtree2.constructTree(features2);

			int height2, interiorNum2, leafNum2;
			qtree2.countHeight(height2);
			qtree2.countNode(interiorNum2, leafNum2);

			if (!(height2 == 11 && interiorNum2 == 1386 && leafNum2 == 4159)) {
				cout << "Case 2: "
					<< "Your answer is height: " << height2
					<< ", interiorNum: " << interiorNum2
					<< ", leafNum: " << leafNum2
					<< " for case2, but the answer is height: 11, interiorNum: "
					"1386, leafNum: 4159\n";
				--cct;
			}

			features2.clear();
			for (size_t i = 0; i < geom2.size(); ++i)
				delete geom2[i];
			geom2.clear();

			cout << "QuadTree Construction: " << cct << " / " << ncase
				<< " tests are passed" << endl;
		}
		else if (t == TEST6) {
			cout << "测试6: Spatial Join" << endl;
			cout << "找出每条道路100m内的公共自行车站点" << endl;
			vector<Geometry*> geom = readGeom(PROJ_SRC_DIR "/data/highway");
			
			vector<Feature> roads;
			roads.clear();
			for (size_t i = 0; i < geom.size(); ++i)
				roads.push_back(hw6::Feature(to_string(i), geom[i]));
			cout << "道路数据读取完毕" << endl;
			geom.clear();

			geom = readGeom(PROJ_SRC_DIR "/data/station");
			vector<string> name = readName(PROJ_SRC_DIR "/data/station");
			vector<Feature> stations;
			for (size_t i = 0; i < geom.size(); ++i)
				stations.push_back(Feature(name[i], geom[i]));
			cout << "站点数据读取完毕" << endl;

			QuadTree roadtree;
			roadtree.setCapacity(60);
			roadtree.constructTree(roads);

			QuadTree pointtree;
			pointtree.setCapacity(60);
			pointtree.constructTree(stations);

			vector<pair<Feature, Feature>> PointJoinRoad;
			vector<pair<Feature, Feature>> RoadJoinPoint;
			//cout << "PointJoinRoad查询" << endl;
			//pointtree.spatialJoin(100, roads, PointJoinRoad,POINTJOINLINE);
			cout << "RoadJoinPoint查询" << endl;
			clock_t start_time = clock();
			roadtree.spatialJoin(10, stations, RoadJoinPoint,LINEJOINPOINT);
			clock_t end_time = clock();
			cout << "查询耗时:" << (end_time - start_time) / 1000.0 << "s" << endl;
		}
		else if (t == TEST8) {
			cout << "测试8: QuadTreeAnalysis" << endl;
			analyse();
		}
		else if (t == TEST7) {
			cout << "测试7: Polygon Spatial Join" << endl;
			cout << "找出每个多边形100m内的公共自行车站点" << endl;

			vector<Geometry*> geom = readGeom(PROJ_SRC_DIR "/data/station");
			vector<string> name = readName(PROJ_SRC_DIR "/data/station");
			vector<Feature> stations;
			for (size_t i = 0; i < geom.size(); ++i)
				stations.push_back(Feature(name[i], geom[i]));
			cout << "站点数据读取完毕:" << geom.size()<<endl;
			geom.clear();

			geom = readGeom(PROJ_SRC_DIR "/data/polygon1");
			vector<Feature> polygons;
			polygons.clear();
			for (size_t i = 0; i < geom.size(); ++i)
				polygons.push_back(hw6::Feature("polygon" + std::to_string(i), geom[i]));
			cout << "polygon数据读取完毕，有"<< geom.size() << endl;

			QuadTree pointtree;
			pointtree.setCapacity(60);
			pointtree.constructTree(stations);

			QuadTree polygontree;
			polygontree.setCapacity(20);
			polygontree.constructTree(polygons);

			vector<pair<Feature, Feature>> PolygonJoinPoint;

			cout << "polygon join point查询" << endl;
			if (polygontree.root == nullptr)
				cout << "nullptr\n";
			clock_t start_time = clock();
			polygontree.spatialJoin(10, stations, PolygonJoinPoint, POLYGONJOINPOINT);
			clock_t end_time = clock();
			cout << "查询耗时:" << (end_time - start_time) / 1000.0 << "s" << endl;
		}

		cout << "**********************End**********************" << endl;
	}



} // namespace hw6