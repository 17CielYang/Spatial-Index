
#include "Common.h"
#include "RTree.h"
#include <ctime>
#include "CMakeIn.h"
#include <cmath>

using namespace hw6;

extern int mode;
extern std::vector<Geometry *> readGeom(const char *filename);
extern std::vector<std::string> readName(const char *filename);
extern void transformValue(double &res, const char *format);
extern void wrongMessage(Envelope e1, Envelope e2, bool cal);
extern void wrongMessage(const Point &pt1, const Point &pt2, double dis,
						 double res);
extern void wrongMessage(Envelope e1, Envelope e2, Envelope cal, Envelope res);

namespace hw6
{

	void RTree::test(int t)
	{
		using namespace std;

		std::cout << "*********************Start*********************" << std::endl;
		if (t == TEST1)
		{
			std::cout << "TEST1: Envelope Contain, Intersect, and Union" << endl;

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

			for (size_t i = 0; i < tests.size(); ++i)
			{
				if (e1.contain(tests[i]) != (i == 0))
				{
					failedCase += 1;
					wrongMessage(e1, tests[i], (i != 0));
				}
				if (tests[i].contain(e1) == true)
				{
					failedCase += 1;
					wrongMessage(tests[i], e1, true);
				}
			}
			cout << "Envelope Contain: " << tests.size() * 2 - failedCase << " / "
				 << tests.size() * 2 << " tests are passed" << endl;

			failedCase = 0;
			for (size_t i = 0; i < tests.size(); ++i)
			{
				if (e1.intersect(tests[i]) != (i < 6))
				{
					failedCase += 1;
					wrongMessage(e1, tests[i], (i < 6));
				}
				if (tests[i].intersect(e1) != (i < 6))
				{
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
			for (size_t i = 0; i < tests.size(); ++i)
			{
				if (e1.unionEnvelope(tests[i]) != results[i])
				{
					failedCase += 1;
					wrongMessage(e1, tests[i], e1.unionEnvelope(tests[i]),
								 results[i]);
				}
				if (tests[i].unionEnvelope(e1) != results[i])
				{
					failedCase += 1;
					wrongMessage(tests[i], e1, e1.unionEnvelope(tests[i]),
								 results[i]);
				}
			}
			cout << "Envelope Union: " << tests.size() * 2 - failedCase << " / "
				 << tests.size() * 2 << " tests are passed" << endl;
		}
		else if (t == TEST2)
		{
			cout << "TEST2: Distance between Point and LineString" << endl;

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

			double dists[] = {0, 0, 14.1421, 14.1421, 0,
							  7.07107, 14.1421, 7.07107, 14.1421, 14.1421};

			int failedCase = 0;
			for (size_t i = 0; i < points.size(); ++i)
			{
				double dist = points[i].distance(&line);
				if (fabs(dist - dists[i]) > 0.0001)
				{
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
		else if (t == TEST3)
		{
			cout << "TEST3: Distance between Point and Polygon" << endl;

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

			double dists[] = {1.26491, 0, 0, 0, 1.48556, 1.58114, 0, 1, 0, 0};

			int failedCase = 0;
			for (size_t i = 0; i < points.size(); ++i)
			{
				double dist = points[i].distance(&poly);
				if (fabs(dist - dists[i]) > 0.00001)
				{
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
		else if (t == TEST4)
		{
			cout << "TEST4: RTree Construction" << endl;
			int ncase, cct;
			ncase = cct = 2;

			RTree rtree(8);
			vector<Geometry *> geom = readGeom(PROJ_SRC_DIR "/data/station");
			vector<Feature> features;

			for (size_t i = 0; i < geom.size(); ++i)
				features.push_back(Feature("", geom[i]));

			rtree.constructTree(features);

			int height, interiorNum, leafNum;
			rtree.countHeight(height);
			rtree.countNode(interiorNum, leafNum);

			if (!(height == 4 && interiorNum == 18 && leafNum == 86))
			{
				cout << "Case 1: "
					 << "Your answer is height: " << height
					 << ", interiorNum: " << interiorNum << ", leafNum: " << leafNum
					 << ". One possible answer is height: 4, interiorNum: "
						"18, "
						"leafNum: 86\n";
				--cct;
			}

			features.clear();
			for (size_t i = 0; i < geom.size(); ++i)
				delete geom[i];
			geom.clear();

			vector<Geometry *> geom2 = readGeom(PROJ_SRC_DIR "/data/highway");
			vector<Feature> features2;
			RTree rtree2(8);

			for (size_t i = 0; i < geom2.size(); ++i)
				features2.push_back(Feature("", geom2[i]));

			rtree2.constructTree(features2);

			int height2, interiorNum2, leafNum2;
			rtree2.countHeight(height2);
			rtree2.countNode(interiorNum2, leafNum2);

			if (!(height2 == 6 && interiorNum2 == 484 && leafNum2 == 2305))
			{
				cout << "Case 2: "
					 << "Your answer is height: " << height2
					 << ", interiorNum: " << interiorNum2
					 << ", leafNum: " << leafNum2
					 << ". One possible answer is height: 6, interiorNum: "
						"484, leafNum: 2305\n";
				--cct;
			}

			features2.clear();
			for (size_t i = 0; i < geom2.size(); ++i)
				delete geom2[i];
			geom2.clear();

			// cout << "RTree Construction: " << cct << " / " << ncase
			//      << " tests are passed" << endl;
		}
		else if (t == TEST5) {
			cout << "TEST5: Distance between LineString and Polygon" << endl;

			vector<Point> points;
			points.push_back(Point(5, 0));
			points.push_back(Point(3, 6));
			points.push_back(Point(2, 4));
			points.push_back(Point(-2, 4));
			points.push_back(Point(-3, 5));
			points.push_back(Point(-5, 0));
			points.push_back(Point(0, -3));
			points.push_back(Point(5, 0));
			LineString exRing(points);

			vector<LineString> inRings;
			points.clear();
			points.push_back(Point(2, 0));
			points.push_back(Point(0, -1));
			points.push_back(Point(-1, 0));
			points.push_back(Point(0, 1));
			points.push_back(Point(2, 0));
			LineString inRing1(points);
			inRings.push_back(inRing1);

			points.clear();
			points.push_back(Point(3, 1));
			points.push_back(Point(2, 1));
			points.push_back(Point(2, 2));
			points.push_back(Point(3, 2));
			points.push_back(Point(3, 1));
			LineString inRing2(points);
			inRings.push_back(inRing2);

			points.clear();
			points.push_back(Point(-2, 1));
			points.push_back(Point(-2, 0));
			points.push_back(Point(-3, 0));
			points.push_back(Point(-3, 1));
			points.push_back(Point(-2, 1));
			LineString inRing3(points);
			inRings.push_back(inRing3);

			Polygon poly(exRing, inRings);

			inRings.clear();
			points.clear();
			points.push_back(Point(-5, -4));
			points.push_back(Point(3, -4));
			LineString line1(points);
			inRings.push_back(line1);

			points.clear();
			points.push_back(Point(1, 7));
			points.push_back(Point(3, 7));
			points.push_back(Point(4, 7));
			LineString line2(points);
			inRings.push_back(line2);

			points.clear();
			points.push_back(Point(3, 0));
			points.push_back(Point(3, 5));
			LineString line3(points);
			inRings.push_back(line3);

			points.clear();
			points.push_back(Point(-4, -4));
			points.push_back(Point(-4, 5));
			LineString line4(points);
			inRings.push_back(line4);

			points.clear();
			points.push_back(Point(0, 0));
			points.push_back(Point(1, 0));
			LineString line5(points);
			inRings.push_back(line5);


			points.clear();
			points.push_back(Point(-2, 0));
			points.push_back(Point(-3, 1));
			LineString line6(points);
			inRings.push_back(line6);

			points.clear();
			points.push_back(Point(-1, 4));
			points.push_back(Point(0, 6));
			LineString line7(points);
			inRings.push_back(line7);

			double dists[] = { 1, 1, 0, 0, 0, 1.0/sqrt(5), 0};

			int failedCase = 0;
			for (size_t i = 0; i < inRings.size(); ++i) {
				//cout << i << endl;
				double dist = inRings[i].distance(&poly);
				if (fabs(dist - dists[i]) > 0.0001) {
					failedCase += 1;
					cout << "Your answer is " << dist << " for test between ";
					poly.print();
					cout << " and ";
					inRings[i].print();
					cout << ", but the answer is " << dists[i] << endl;
				}
			}
			cout << "Distance between LineString and Polygon: "
				<< inRings.size() - failedCase << " / " << inRings.size()
				<< " tests are passed" << endl;
		}
		else if (t == TEST6) {
			cout << "test6:Spatial Join" << endl;
			//cout << "找出每条道路100m内的公共自行车站点" << endl;
			vector<Geometry*> geom = readGeom(PROJ_SRC_DIR "/data/highway");

			vector<Feature> roads;
			roads.clear();
			for (size_t i = 0; i < geom.size(); ++i)
				roads.push_back(hw6::Feature(to_string(i), geom[i]));
			cout << "road data lode" << endl;
			geom.clear();

			geom = readGeom(PROJ_SRC_DIR "/data/station");
			vector<string> name = readName(PROJ_SRC_DIR "/data/station");
			vector<Feature> stations;
			for (size_t i = 0; i < geom.size(); ++i)
				stations.push_back(Feature(name[i], geom[i]));
			cout << "station data lode" << endl;

			RTree roadtree(60);
			//roadtree.setCapacity(60);
			roadtree.constructTree(roads);

			RTree pointtree(60);
			//pointtree.setCapacity(20);
			pointtree.constructTree(stations);

			vector<pair<Feature, Feature>> PointJoinRoad;
			vector<pair<Feature, Feature>> RoadJoinPoint;
			//cout << "PointJoinRoad查询" << endl;
			//pointtree.spatialJoin(100, roads, PointJoinRoad,POINTJOINLINE);
			cout << "RoadJoinPoint query" << endl;
			clock_t start_time = clock();
			roadtree.spatialJoin(100, &pointtree, RoadJoinPoint, LINEJOINPOINT);
			clock_t end_time = clock();
			cout << "query time:" << (end_time - start_time) / 1000.0 << "s" << endl;
		}
		else if (t == TEST8)
		{
			cout << "TEST8: RTreeAnalysis" << endl;
			analyse();
		}

		cout << "**********************End**********************" << endl;
	}

	void forConstCapAnalyseRTree(const std::vector<Feature> &features, int childNum, int maxNum, int step)
	{
		using namespace std;
		if (childNum <= maxNum)
		{
			RTree rtree(childNum);
			//rtree.constructTree(features);

			// TODO 与四叉树进行比较

			clock_t start_time = clock();
			// TODO
			rtree.constructTree(features);
			clock_t end_time = clock();

			int height = 0, interiorNum = 0, leafNum = 0;
			// TODO
			rtree.countHeight(height);
			rtree.countNode(interiorNum, leafNum);

			cout << "Capacity " << rtree.getCapacity() << "\n";
			cout << "Height: " << height
				<< " \tInterior node number: " << interiorNum
				<< " \tLeaf node number: " << leafNum << "\n";
			cout << "Construction time: " << (end_time - start_time) / 1000.0 << "s"
				<< endl;

			double x, y;
			vector<Feature> candidateFeatures;
			//hw6::Feature nearestFeature;
			start_time = clock();
			for (int i = 0; i < 100; ++i) {
				//candidateFeatures.clear();
				x = -((rand() % 225) / 10000.0 + 73.9812);
				y = (rand() % 239) / 10000.0 + 40.7247;
				rtree.NNQuery(x, y, candidateFeatures);
				// refine step
				// TODO
				double minDis = std::numeric_limits<double>::infinity();
				for (size_t j = 0; j < candidateFeatures.size(); ++j) {
					double dist = candidateFeatures[j].distance(x, y);
					if (j == 0 || dist < minDis) {
						minDis = dist;
						//nearestFeature = candidateFeatures[i];
					}
				}
				if ((i + 1) % 10000 == 0) {
					cout << "NNQuery " << i + 1 << " times" << endl;
				}
			}
			end_time = clock();
			cout << "NNQuery time: " << (end_time - start_time) / 1000.0 << "s"
				<< endl
				<< endl;

			forConstCapAnalyseRTree(features, childNum + step, maxNum, step);
		}
	}

	void RTree::analyse()
	{
		using namespace std;

		vector<Feature> features;
		vector<Geometry *> geom = readGeom(PROJ_SRC_DIR "/data/taxi");
		vector<string> name = readName(PROJ_SRC_DIR "/data/taxi");

		features.clear();
		features.reserve(geom.size());
		for (size_t i = 0; i < geom.size(); ++i)
			features.push_back(Feature(name[i], geom[i]));

		cout << "taxi number: " << geom.size() << endl;

		srand(time(nullptr));

		/*TODO:实现forConstCapAnalyseRTree */
		forConstCapAnalyseRTree(features, 40, 200, 10);
	}

} // namespace hw6