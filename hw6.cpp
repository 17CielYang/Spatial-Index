﻿// hw6.cpp : 定义控制台应用程序的入口点。
//

#include "Common.h"
#include "Geometry.h"
#include "shapelib/shapefil.h"
#include "Tree.h"

#include "CMakeIn.h" // 配置注入

#include <gl/freeglut.h> // GLUT库头文件

#include <cstdio>
#include <ctime>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <vector>

#define USE_RTREE

#ifdef USE_RTREE
#include "RTree.h"
using TreeTy = hw6::RTree;
#else
#include "QuadTree.h"
using TreeTy = hw6::QuadTree;
#endif

using namespace std;

int screenWidth = 640;
int screenHeight = 480;
int kvalue=10;

double pointSize = 2.0;

int mode;

vector<hw6::Feature> features;
vector<hw6::Feature> roads;
vector<hw6::Feature> polygons;
bool showRoad = true;
bool showPolygon = false;
//unique_ptr<hw6::Tree> stationTree;
unique_ptr<hw6::Tree> roadTree;
//unique_ptr<hw6::Tree> texiTree;
unique_ptr<hw6::Tree> pointTree;
unique_ptr<hw6::Tree> polygonTree;
bool showTree = false;

hw6::Feature nearestFeature;


bool firstPoint = true;
hw6::Point corner[2];
hw6::Envelope selectedRect;
vector<hw6::Feature> selectedFeatures;
vector<hw6::Feature> nearestkFeatures;

/*
 * shapefile文件中name和geometry属性读取
 */
vector<string> readName(const char* filename) {
	DBFHandle file = DBFOpen(filename, "r");

	vector<string> res;
	int cct = DBFGetRecordCount(file);
	res.reserve(cct);
	for (int i = 0; i < cct; ++i) {
		string a = DBFReadStringAttribute(file, i, 0);
		res.push_back(a);
	}

	DBFClose(file);

	return res;
}

vector<hw6::Geometry*> readGeom(const char* filename) {
	SHPHandle file = SHPOpen(filename, "r");

	int pnEntities, pnShapeType;
	double padfMinBound[4], padfMaxBound[4];
	SHPGetInfo(file, &pnEntities, &pnShapeType, padfMinBound, padfMaxBound);

	vector<hw6::Point> points;
	vector<hw6::Geometry*> geoms;
	geoms.reserve(pnEntities);
	switch (pnShapeType) {
	case SHPT_POINT:
		for (int i = 0; i < pnEntities; ++i) {
			SHPObject* pt = SHPReadObject(file, i);
			geoms.push_back(new hw6::Point(pt->padfY[0], pt->padfX[0]));
			SHPDestroyObject(pt);
		}
		break;

	case SHPT_ARC:
		for (int i = 0; i < pnEntities; ++i) {
			points.clear();
			SHPObject* pt = SHPReadObject(file, i);
			for (int j = 0; j < pt->nVertices; ++j) {
				points.push_back(hw6::Point(pt->padfY[j], pt->padfX[j]));
			}
			SHPDestroyObject(pt);
			geoms.push_back(new hw6::LineString(points));
		}
		break;

	case SHPT_POLYGON:
		for (int i = 0; i < pnEntities; ++i) {
			points.clear();
			SHPObject* pt = SHPReadObject(file, i);
			for (int j = 0; j < pt->nVertices; ++j) {
				points.push_back(hw6::Point(pt->padfY[j], pt->padfX[j]));
			}
			SHPDestroyObject(pt);
			hw6::LineString line(points);
			hw6::Polygon* poly = new hw6::Polygon(line);
			geoms.push_back(new hw6::Polygon(line));
		}
		break;
	}

	SHPClose(file);
	return geoms;
}

void transformValue(double& res, const char* format = "%.2lf") {
	char buf[20];
	sprintf(buf, format, res);
	sscanf(buf, "%lf", &res);
}

void wrongMessage(hw6::Envelope e1, hw6::Envelope e2, bool cal) {
	cout << "Your answer is " << cal << " for test between ";
	e1.print();
	cout << " and ";
	e2.print();
	cout << ", but the answer is " << !cal << endl;
}

void wrongMessage(const hw6::Point& pt1, const hw6::Point& pt2, double dis,
	double res) {
	cout << "Your answer is " << dis << " for test ";
	pt1.print();
	cout << " and ";
	pt2.print();
	cout << ", but the answer is " << res << endl;
}

void wrongMessage(hw6::Envelope e1, hw6::Envelope e2, hw6::Envelope cal,
	hw6::Envelope res) {
	cout << "Your answer is ";
	cal.print();
	cout << " for test between ";
	e1.print();
	cout << " and ";
	e2.print();
	cout << ", but the answer is ";
	res.print();
	cout << endl;
}

/*
 * 输出几何信息
 */
void printGeom(vector<hw6::Geometry*>& geom) {
	cout << "Geometry:" << endl;
	for (vector<hw6::Geometry*>::iterator it = geom.begin(); it != geom.end();
		++it) {
		(*it)->print();
	}
}

/*
 * 删除几何信息
 */
void deleteGeom(vector<hw6::Geometry*>& geom) {
	for (vector<hw6::Geometry*>::iterator it = geom.begin(); it != geom.end();
		++it) {
		delete* it;
		*it = NULL;
	}
	geom.clear();
}

/*
 * 读取纽约道路数据
 */
void loadRoadData() {
	vector<hw6::Geometry*> geom = readGeom(PROJ_SRC_DIR "/data/highway");

	roads.clear();
	for (size_t i = 0; i < geom.size(); ++i)
		roads.push_back(hw6::Feature(to_string(i), geom[i]));

	cout << "road number: " << geom.size() << endl;
	roadTree->setCapacity(20);
	roadTree->constructTree(roads);
}

/*
 * 读取纽约自行车租赁点数据
 */
void loadStationData() {
	vector<hw6::Geometry*> geom = readGeom(PROJ_SRC_DIR "/data/station");
	vector<string> name = readName(PROJ_SRC_DIR "/data/station");
	cout << "read station" << endl;
	features.clear();
	for (size_t i = 0; i < geom.size(); ++i) 
		features.push_back(hw6::Feature(name[i], geom[i]));

	cout << "station number: " << geom.size() << endl;
	pointTree->setCapacity(5);
	pointTree->constructTree(features);
}

/*
 * 读取纽约出租车打车点数据
 */
void loadTaxiData() {
	vector<hw6::Geometry*> geom = readGeom(PROJ_SRC_DIR "/data/taxi");
	vector<string> name = readName(PROJ_SRC_DIR "/data/taxi");

	features.clear();
	for (size_t i = 0; i < geom.size(); ++i)
		features.push_back(hw6::Feature(name[i], geom[i]));

	cout << "taxi number: " << geom.size() << endl;
	pointTree->setCapacity(100);
	pointTree->constructTree(features);
}
/*
 * 读取polygon
 */
void loadPolygonData() {
	vector<hw6::Geometry*> geom = readGeom(PROJ_SRC_DIR "/data/polygon1");
	//vector<string> name = readName(PROJ_SRC_DIR "/data/polygon");
	polygons.clear();
	for (size_t i = 0; i < geom.size(); ++i)
		polygons.push_back(hw6::Feature("polygon" + std::to_string(i), geom[i]));

	cout << "polygon number: " << geom.size() << endl;
	polygonTree->setCapacity(6);
	polygonTree->constructTree(polygons);
}


/*
 * 区域查询
 */
//feature列表去重
void uniqueFeatures(std::vector<hw6::Feature>& features) {
	std::set<std::string> uniqueNames;
	std::vector<hw6::Feature> uniqueFeatures;

	for (const auto& feature : features) {
		if (uniqueNames.insert(feature.getName()).second) {
			uniqueFeatures.push_back(feature);
		}
	}

	features.swap(uniqueFeatures); // 用去重后的特征列表替换原始列表
}

void rangeQuery() {
	vector<hw6::Feature> candidateFeatures;

	// filter step (使用四叉树获得查询区域和几何特征包围盒相交的候选集）
	if (mode == RANGEPOINT)
		pointTree->rangeQuery(selectedRect, candidateFeatures);
	else if (mode == RANGELINE)
		roadTree->rangeQuery(selectedRect, candidateFeatures);
	else if (mode == RANGEPOLYGON) {
		polygonTree->rangeQuery(selectedRect, candidateFeatures);
		//cout << "polygon range\n";
	}
	uniqueFeatures(candidateFeatures);
	//把查询结果标红？
	selectedFeatures.clear();
	for (const auto& feature : candidateFeatures) {
		if (mode == RANGEPOINT|| mode == RANGELINE) {
			if (feature.getGeom()->intersects(selectedRect)) {
				selectedFeatures.push_back(feature);
			}
		}else if (mode == RANGEPOLYGON) {
				selectedFeatures.push_back(feature);
		}
	}
	//cout << selectedFeatures.size() << endl;
	// refine step (精确判断时，需要去重，避免查询区域和几何对象的重复计算)
	// TODO
}

/*
 * 邻近查询
 */
void NNQuery(hw6::Point p) {
	vector<hw6::Feature> candidateFeatures;

	// filter step (使用四叉树获得距离较近的几何特征候选集)
	if (mode == NNPOINT)
		pointTree->NNQuery(p.getX(), p.getY(), candidateFeatures);
	else if (mode == NNLINE)
		roadTree->NNQuery(p.getX(), p.getY(), candidateFeatures);
	else if (mode == NNPOLYGON) {
		polygonTree->NNQuery(p.getX(), p.getY(), candidateFeatures);
		//cout << "polygon nn" << endl;
	}
	uniqueFeatures(candidateFeatures);

	double minDist,dist;
	if (candidateFeatures.size() != 0)
	{
		minDist = candidateFeatures[0].distance(p.getX(), p.getY());
		nearestFeature = candidateFeatures[0];

		//把最后查询结果储存进selectFeatures当中
		for (const auto& feature : candidateFeatures) {
			dist = feature.distance(p.getX(), p.getY());
			if (dist < minDist)
				minDist = dist;
			nearestFeature = feature;
		}
	}
	//cout << "结果数量" << selectedFeatures.size() << endl;
	// refine step (精确计算查询点与几何对象的距离)
	// TODO
}
/*spatial join 查询道路100米范围内的站点*/

void KNNQuery(hw6::Point p) {
	//std::cout << "Enter an integer: " << endl;
	//std::cin >> k;
	vector<hw6::Feature> candidateFeatures;
	if (mode == KNNPOINT)
		pointTree->KNNQuery(p.getX(), p.getY(), kvalue, candidateFeatures);
	else if (mode == KNNLINE)
		roadTree->KNNQuery(p.getX(), p.getY(), kvalue, candidateFeatures);
	else if (mode == KNNPOLYGON)
		polygonTree->KNNQuery(p.getX(), p.getY(), kvalue, candidateFeatures);
	nearestkFeatures.clear();

	uniqueFeatures(candidateFeatures);
	for (const auto& feature : candidateFeatures) {
		
			nearestkFeatures.push_back(feature);
	}

}



/*
 * 从屏幕坐标转换到地理坐标
 */
void transfromPt(hw6::Point& pt) {
	const hw6::Envelope bbox = pointTree->getEnvelope();
	double width = bbox.getMaxX() - bbox.getMinX() + 0.002;
	double height = bbox.getMaxY() - bbox.getMinY() + 0.002;

	double x = pt.getX() * width / screenWidth + bbox.getMinX() - 0.001;
	double y = pt.getY() * height / screenHeight + bbox.getMinY() - 0.001;

	x = max(bbox.getMinX(), x);
	x = min(bbox.getMaxX(), x);
	y = max(bbox.getMinY(), y);
	y = min(bbox.getMaxY(), y);
	pt = hw6::Point(x, y);
}

/*
 * 绘制代码
 */
void display() {
	// glClearColor(241 / 255.0, 238 / 255.0, 232 / 255.0, 0.0);
	// 设置背景颜色
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	const hw6::Envelope bbox = pointTree->getEnvelope();
	glOrtho(bbox.getMinX() - 0.001, bbox.getMaxX() + 0.001,
		bbox.getMinY() - 0.001, bbox.getMaxY() + 0.001,
		-1.0, 1.0);

	// 道路绘制
	if (showRoad) {
		glColor3d(252 / 255.0, 214 / 255.0, 164 / 255.0);
		//glColor3d(0.6, 0.3, 0.0); // 棕色
		for (size_t i = 0; i < roads.size(); ++i)
			roads[i].draw();
	}

	// 点绘制
	if (!(mode == RANGELINE || mode == NNLINE)) {
		glPointSize((float)pointSize);
		glColor3d(0.0, 146 / 255.0, 247 / 255.0);
		for (size_t i = 0; i < features.size(); ++i)
			features[i].draw();
	}
	if (showPolygon) {
		glColor3d(0.0, 146 / 255.0, 247 / 255.0); // 红色
		for (size_t i = 0; i < polygons.size(); ++i)
			polygons[i].draw();
	}

	// 四叉树绘制
	if (showTree) {
		glColor3d(0.0, 146 / 255.0, 247 / 255.0);
		if (mode == RANGELINE || mode == NNLINE)
			roadTree->draw();
		else
			pointTree->draw();
	}

	// 离鼠标最近点绘制
	if (mode == NNPOINT) {
		glPointSize(5.0);
		glColor3d(0.9, 0.0, 0.0);
		nearestFeature.draw();
	}

	// 离鼠标最近道路绘制
	if (mode == NNLINE) {
		glLineWidth(3.0);
		glColor3d(0.9, 0.0, 0.0);
		nearestFeature.draw();
		glLineWidth(1.0);
	}
	if (mode == NNPOLYGON) {
		glLineWidth(3.0);
		glColor3d(0.9, 0.0, 0.0);
		nearestFeature.draw();
		glLineWidth(1.0);
	}
	if (mode == KNNPOINT) {
		glPointSize(5.0);
		glColor3d(0.9, 0.0, 0.0);
		for (size_t i = 0; i < nearestkFeatures.size(); ++i)
			nearestkFeatures[i].draw();
	}
	if (mode == KNNLINE||mode==KNNPOLYGON) {
		glLineWidth(3.0);
		glColor3d(0.9, 0.0, 0.0);
		for (size_t i = 0; i < nearestkFeatures.size(); ++i)
			nearestkFeatures[i].draw();
		glLineWidth(1.0);
	}

	// 区域选择绘制
	if (mode == RANGEPOINT || mode == RANGELINE||mode==RANGEPOLYGON) {
		glColor3d(0.0, 0.0, 0.0);
		selectedRect.draw();
		glColor3d(1.0, 0.0, 0.0);
		
		for (size_t i = 0; i < selectedFeatures.size(); ++i){
			selectedFeatures[i].draw();
			cout << "draw\n";
		}
		//glLineWidth(1.0);
	}

	glFlush();
	glutSwapBuffers();
}

/*
 * 鼠标和键盘交互
 */
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (mode == RANGEPOINT || mode == RANGELINE||mode==RANGEPOLYGON) {
			if (firstPoint) {
				selectedFeatures.clear();
				corner[0] = hw6::Point(x, screenHeight - y);
				transfromPt(corner[0]);
			}
			else {
				corner[1] = hw6::Point(x, screenHeight - y);
				transfromPt(corner[1]);
				selectedRect =
					hw6::Envelope(min(corner[0].getX(), corner[1].getX()),
						max(corner[0].getX(), corner[1].getX()),
						min(corner[0].getY(), corner[1].getY()),
						max(corner[0].getY(), corner[1].getY()));
				rangeQuery();
			}
			firstPoint = !firstPoint;
			glutPostRedisplay();
		}
	}
}

void passiveMotion(int x, int y) {
	corner[1] = hw6::Point(x, screenHeight - y);

	if ((mode == RANGEPOINT || mode == RANGELINE||mode==RANGEPOLYGON) && !firstPoint) {
		corner[1] = hw6::Point(x, screenHeight - y);
		transfromPt(corner[1]);
		//cout << "range" << endl;
		selectedRect = hw6::Envelope(min(corner[0].getX(), corner[1].getX()),
			max(corner[0].getX(), corner[1].getX()),
			min(corner[0].getY(), corner[1].getY()),
			max(corner[0].getY(), corner[1].getY()));
		//if (mode == RANGEPOLYGON) cout << "polygon range\n";
		rangeQuery();

		glutPostRedisplay();
	}
	else if (mode == NNPOINT || mode == NNLINE||mode==NNPOLYGON) {
		hw6::Point p(x, screenHeight - y);
		transfromPt(p);
		NNQuery(p);

		glutPostRedisplay();
	}
	else if (mode == KNNPOINT || mode == KNNLINE||mode==KNNPOLYGON) {
		hw6::Point p(x, screenHeight - y);
		transfromPt(p);
		KNNQuery(p);

		glutPostRedisplay();
	}
}

void changeSize(int w, int h) {
	screenWidth = w;
	screenHeight = h;
	glViewport(0, 0, w, h);
	glutPostRedisplay();
}

void processNormalKeys(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		exit(0);
		break;
	case 'N':
		mode = NNLINE;
		break;
	case 'n':
		mode = NNPOINT;
		break;
	case 'S':
		mode = RANGELINE;
		firstPoint = true;
		break;
	case 's':
		mode = RANGEPOINT;
		firstPoint = true;
		break;
	case 'K':
		mode = KNNLINE;
		cout << "please enter k:";
		cin >> kvalue;
		cout << "finish";
		break;
	case 'k':
		mode = KNNPOINT;
		cout << "please enter k:";
		cin >> kvalue;
		cout << "finish";
		break;
	case 'B':
	case 'b':
		loadStationData();
		mode = Default;
		break;
	case 'T':
	case 't':
		loadTaxiData();
		mode = Default;
		break;
	case 'R':
	case 'r':
		showRoad = !showRoad;
		break;
	case 'Q':
	case 'q':
		showTree = !showTree;
		break;
	case 'P':
	case 'p':
		loadPolygonData();
		showPolygon = !showPolygon;
		mode = Default;
		break;
	case 'Z':
		mode = RANGEPOLYGON;
		firstPoint = true;
		break;
	case 'z':
		mode = NNPOLYGON;
		break;
	case 'x':
		mode = KNNPOLYGON;
		cout << "please enter k:";
		cin >> kvalue;
		cout << "finish";
		break;
	case '+':
		pointSize *= 1.1;
		break;
	case '-':
		pointSize /= 1.1;
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
		if(!(mode == KNNPOINT||mode==KNNLINE||mode==KNNPOLYGON))
			TreeTy::test(key - '0');
		break;
	default:
		mode = Default;
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char* argv[]) {
	cout << "Key Usage:\n"
		<< "  S  : range search for roads\n"
		<< "  s  : range search for stations\n"
		<< "  N  : nearest road search\n"
		<< "  n  : nearest station search\n"
		<< "  K  : nearest k road search\n"
		<< "  k  : nearest k stations search\n"
		<< "  B/b: Bicycle data\n"
		<< "  T/t: Taxi data\n"
		<< "  R/r: show Road\n"
		<< "  Q/q: show Tree\n"
		<< "  P/p: show Polygon\n"
		<< "  Z  : range search for polygons\n"
		<< "  z  : nearest polygon search\n"
		<< "  x  : nearest k polygons search\n"
		<< "  +  : increase point size\n"
		<< "  -  : decrease point size\n"
		<< "  1  : Test Envelope contain, interset and union\n"
		<< "  2  : Test distance between Point and LineString\n"
		<< "  3  : Test distance between Point and Polygon\n"
		<< "  4  : Test tree construction\n"
		<< "  5  : Test (your option here)\n"
		<< "  8  : Tree performance analysis\n"
		<< "  ESC: quit\n"
		<< endl;

	pointTree = make_unique<TreeTy>();
	roadTree = make_unique<TreeTy>();
	polygonTree= make_unique<TreeTy>();
	cout << "read road data" << endl;
	loadRoadData();
	cout << "read road data done" << endl;
	loadStationData();
	loadPolygonData();

	pointTree->getEnvelope().print();
	
		glutInit_ATEXIT_HACK(&argc, argv);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("New York");

	glutMouseFunc(mouse);
	glutDisplayFunc(display);
	glutPassiveMotionFunc(passiveMotion);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(processNormalKeys);

	glutMainLoop();

	return 0;
}