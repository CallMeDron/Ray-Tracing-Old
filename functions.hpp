#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "CImg.h"
#include <cmath>
using namespace std;

vector<double> GetSettings();
vector<Figure*> GetFigures();
int RayTracing(vector<double> settings, vector<Figure*> figures);
vector<int> GetColor(int x, int y, vector<double> settings, vector<Figure*> figures);
vector<double> CrossSphere(vector<double> cam, vector<double> direction_norm, vector<Figure*> figures, vector<double> settings);
vector<int> ShadowSphere(vector<Figure*> figures, vector<int> color, vector<double> Spoint, vector<double> light);
double ShadowBox(vector<double> point, vector<double> light, vector<double> edge1, vector<double>edge5, vector<double>edge4, vector<double>edge8);
vector<double> CrossBox(vector<double> cam, vector<double> direction_norm, vector<Figure*> figures, vector<double> settings, vector<double> light);
vector<double> CrossTetra(vector<double> cam, vector<double> direction_norm, vector<Figure*> figures, vector<double> settings, vector<double> light);
double DistanceBtwPoints(vector<double> point1, vector<double> point2);
double Norma(vector<double> vector);
vector<double> Normalize(vector<double> vector);
double Scalar(vector<double> vector1, vector<double> vector2);
vector<double> Vect(vector<double>  normal_norm, vector<double> up_norm);
vector<double> ProjectPointOnLine(vector<double> startpoint, vector<double> direction, vector<double> point);
bool InZone(vector<double> point, vector<double> settings);
vector<double> ProjectPointOnPlane(vector<double> point, vector<double> start, vector<double> normal_norm);
vector<double> CrossRectangle(vector<double> cam, vector<double> direction_norm, vector<double> edge1, vector<double>edge5, vector<double>edge4, vector<double>edge8);
vector<double> CrossTriangle(vector<double> cam, vector<double> direction_norm, vector<double> edge1, vector<double>edge4, vector<double>edge2);
vector<double> CrossLineAndPlane(vector<double> cam, vector<double> direction_norm, vector<double> edge1, vector<double> normal_norm);