#include "Figure.hpp"
#include "functions.hpp"
#include "Factory.hpp"
#include "SphereFactory.hpp"
#include "BoxFactory.hpp"
#include "TetraFactory.hpp"

vector<double> GetSettings()
{
	ifstream in("settings.txt");

	if (!in)
	{
		std::cout << "Settings file doesn't exist" << endl;
		return vector<double>();
	}

	int i = 0;
	vector<double> settings;
	settings.resize(17);
	for (vector<double>::iterator it = settings.begin(); it < settings.end(); it++) *it = 0.0;
	array<string, 9> str = { "" };


	for (i = 0; i < 3; i++)
	{
		if (!(in >> str[i] >> settings[3 * i] >> settings[3 * i + 1] >> settings[3 * i + 2]))
		{
			std::cout << "Settings incorrect" << endl;
			return vector<double>();
		}
	}
	for (i = 0; i < 5; i++)
	{
		if (!(in >> str[i + 3] >> settings[i + 9]))
		{
			std::cout << "Settings incorrect" << endl;
			return vector<double>();
		}
	}
	if (!(in >> str[8] >> settings[14] >> settings[15] >> settings[16]))
	{
		std::cout << "Settings incorrect" << endl;
		return vector<double>();
	}

	if (str[0] != "cam" || str[1] != "normal" || str[2] != "up" || str[3] != "distance" || str[4] != "limit" || str[5] != "alpha" || str[6] != "width" || str[7] != "height" || str[8] != "light"
		|| (settings[3] == 0.0 && settings[4] == 0.0 && settings[5] == 0.0)
		|| (settings[6] == 0.0 && settings[7] == 0.0 && settings[8] == 0.0)
		|| (settings[3] * settings[6] + settings[4] * settings[7] + settings[5] * settings[8] != 0)
		|| settings[9] <= 0.0
		|| settings[10] <= settings[9]
		|| settings[11] <= 0.0 || settings[11] >= 180.0)
	{
		std::cout << "Settings incorrect" << endl;
		return vector<double>();
	}

	in.close();

	return settings;
}

vector<Figure*> GetFigures()
{
	ifstream in("figures.txt");

	if (!in)
	{
		std::cout << "Figures file doesn't exist" << endl;
		return vector<Figure*>();
	}

	vector<Figure*> figures;

	SphereFactory Sphere;
	BoxFactory Box;
	TetraFactory Tetra;
	map<string, Factory*> factories = { {"sphere",&Sphere},{"box",&Box},{"tetra",&Tetra} };

	string str;

	while (!in.eof())
	{
		in >> str;
		if (str != "sphere" && str != "box" && str != "tetra")
		{
			std::cout << "Figures are incorrect" << endl;
			return vector<Figure*>();
		}
		auto ResFactory = factories[str];
		auto NewFigure = ResFactory->Create();
		for (int i = 0; i < NewFigure->dimension; i++)
		{
			if (!(in >> NewFigure->base_data[i]))
			{
				std::cout << "Figures are incorrect" << endl;
				return vector<Figure*>();
			}
		}
		if (str == "sphere" && NewFigure->base_data[3] < 0)
		{
			{
				std::cout << "Sphere is incorrect" << endl;
				return vector<Figure*>();
			}
		}
		if (str == "box")
		{
			vector<double> edge1(3), edge7(3), e1 = { 1,0,0 }, e2 = { 0,1,0 }, e3 = { 0,0,1 }, v(3);
			for (int i = 0; i < 3; i++)
			{
				edge1[i] = NewFigure->base_data[i];
				edge7[i] = NewFigure->base_data[i + 3];
				v[i] = edge7[i] - edge1[i];
			}
			v = Normalize(v);
			if ((Scalar(v, e1) <= 0 || Scalar(v, e1) == 1) || (Scalar(v, e2) <= 0 || Scalar(v, e2) == 1) || (Scalar(v, e3) <= 0 || Scalar(v, e3) == 1))
			{
				std::cout << "Box is incorrect" << endl;
				return vector<Figure*>();
			}
		}
		figures.push_back(NewFigure);
	}

	in.close();

	return figures;
}

int RayTracing(vector<double> settings, vector<Figure*> figures)
{
	int width = int(settings[12]), height = int(settings[13]);
	cimg_library::CImg<unsigned char> img(width, height, 1, 3, 255);
	unsigned char color[3];
	vector<int> arr;

	std::cout << endl << "Wait a minute..." << endl;

#pragma omp parallel for
	for (int x = 0; x < width; x++)
	{
#pragma omp parallel for private(color,arr)
		for (int y = 0; y < height; y++)
		{
			arr = GetColor(x, y, settings, figures);
			color[0] = arr[0];
			color[1] = arr[1];
			color[2] = arr[2];
			img.draw_point(x, y, color);
		}
	}
	//img.display(); // MOZHNO VKLUCHIT' PO ZHELANIU
	img.save("test.bmp");

	std::cout << endl << "Success!" << endl;

	return 0;
}

vector<int> GetColor(int x, int y, vector<double> settings, vector<Figure*> figures)
{
	int width = int(settings[12]), height = int(settings[13]);
	double distance = settings[9], limit = settings[10], alpha = settings[11],
		pi = 3.1415926535,
		metr = (2 * distance * tan(alpha * pi / 360)) / height;

	vector<int> color(3, 255), black(3, 0);

	vector<double> cam = { settings[0], settings[1], settings[2] },
		normal_norm = { settings[3], settings[4], settings[5] },
		up_norm = { settings[6] , settings[7] , settings[8] },
		light = { settings[14], settings[15], settings[16] };

	normal_norm = Normalize(normal_norm);
	up_norm = Normalize(up_norm);

	vector<double> binorm_norm = Vect(up_norm, normal_norm);

	//PREDPOLAGAEM, CHTO VECTOR NORMALI K EKRANU NAPRAVLEN !OT! KAMERI

	vector<double> screen_edge(3);
	for (int k = 0; k < 3; k++)
	{
		screen_edge[k] = cam[k] + distance * normal_norm[k] + height * metr / 2 * up_norm[k] + width * metr / 2 * binorm_norm[k];
	}

	vector<double> cur_screen_point(3);
	for (int k = 0; k < 3; k++)
	{
		cur_screen_point[k] = screen_edge[k] - ((0.5 + x) * binorm_norm[k]) * metr - ((0.5 + y) * up_norm[k]) * metr;
	}

	vector<double> direction_norm(3);
	for (int k = 0; k < 3; k++)
	{
		direction_norm[k] = cur_screen_point[k] - cam[k];
	}

	direction_norm = Normalize(direction_norm);

	vector<double> FPoint;

	vector<double> Spoint = CrossSphere(cam, direction_norm, figures, settings);
	double Spointdist = DistanceBtwPoints(cam, Spoint);
	vector<double> Tpoint = CrossTetra(cam, direction_norm, figures, settings, light);
	double Tpointdist = DistanceBtwPoints(cam, Tpoint);
	vector<double> Bpoint = CrossBox(cam, direction_norm, figures, settings, light);
	double Bpointdist = DistanceBtwPoints(cam, Bpoint);

	if (Spointdist < Tpointdist && Spointdist < Bpointdist)
	{
		color = ShadowSphere(figures, color, Spoint, light);
	}

	if (Bpointdist < Tpointdist && Bpointdist < Spointdist)
	{
		for (auto it = figures.begin(); it < figures.end(); it++)
		{
			if ((*it)->dimension == 6)
			{
				for (int i = 0; i < 3; i++)
				{
					color[i] = (*it)->color[i];
				}
				double k = Bpoint[3];
				if (k < 0) k = 0;
				for (int i = 0; i < 3; i++)
				{
					color[i] = int(color[i] * k);
					if (color[i] > 255) color[i] = 255;
				}
				break;
			}
		}
	}

	if (Tpointdist < Spointdist && Tpointdist < Bpointdist)
	{
		for (auto it = figures.begin(); it < figures.end(); it++)
		{
			if ((*it)->dimension == 12)
			{
				for (int i = 0; i < 3; i++)
				{
					color[i] = (*it)->color[i];
				}
				double k = Tpoint[3];
				if (k < 0) k = 0;
				for (int i = 0; i < 3; i++)
				{
					color[i] = int(color[i] * k);
					if (color[i] > 255) color[i] = 255;
				}
				break;
			}
		}
	}

	return color;
}

vector<double> CrossTetra(vector<double> cam, vector<double> direction_norm, vector<Figure*> figures, vector<double> settings, vector<double> light)
{
	vector<double> edge1(3), edge2(3), edge3(3), edge4(3), v(4);
	double mindist, temp;
	vector<vector<double>> crosslist;

	for (auto it = figures.begin(); it < figures.end(); it++)
	{
		if ((*it)->dimension == 12)
		{
			for (int i = 0; i < 3; i++)
			{
				edge1[i] = (*it)->base_data[i];
				edge2[i] = (*it)->base_data[i + 3];
				edge3[i] = (*it)->base_data[i + 6];
				edge4[i] = (*it)->base_data[i + 9];
			}
			break;
		}
	}

	v = CrossTriangle(cam, direction_norm, edge1, edge4, edge2);
	if (!v.empty() && InZone(v, settings))
	{
		v[3] = ShadowBox(v, light, edge1, edge4, edge2, edge3);
		crosslist.push_back(v);
	}
	v = CrossTriangle(cam, direction_norm, edge1, edge4, edge3);
	if (!v.empty() && InZone(v, settings))
	{
		v[3] = ShadowBox(v, light, edge1, edge4, edge3, edge2);
		crosslist.push_back(v);
	}
	v = CrossTriangle(cam, direction_norm, edge3, edge4, edge2);
	if (!v.empty() && InZone(v, settings))
	{
		v[3] = ShadowBox(v, light, edge3, edge4, edge2, edge1);
		crosslist.push_back(v);
	}
	v = CrossTriangle(cam, direction_norm, edge1, edge2, edge3);
	if (!v.empty() && InZone(v, settings))
	{
		v[3] = ShadowBox(v, light, edge1, edge2, edge3, edge4);
		crosslist.push_back(v);
	}

	if (!crosslist.empty())
	{
		v = (*crosslist.begin());
		mindist = DistanceBtwPoints(cam, v);
		for (auto it = crosslist.begin(); it < crosslist.end(); it++)
		{
			temp = DistanceBtwPoints(cam, *it);
			if (abs(v[0] - (*it)[0]) < 1e5 && abs(v[1] - (*it)[1]) < 1e5 && abs(v[2] - (*it)[2]) < 1e5)
			{
				v[3] = max((*it)[3], v[3]);
			}
			else
			{
				if (temp < mindist)
				{
					mindist = temp;
					v = (*it);
				}
			}
		}
		return v;
	}

	return vector<double>(0);
}

vector<double> CrossTriangle(vector<double> cam, vector<double> direction_norm, vector<double> edge1, vector<double>edge4, vector<double>edge2)
{
	vector<double> cross(4), v1(3), v2(3), v3(3), normal_norm(3),
		prpoint1(3), prpoint2(3), prpoint3(3), prpoint11(3), prpoint22(3), prpoint33(3),
		prvec1(3), prvec2(3), prvec3(3), prvec11(3), prvec22(3), prvec33(3);

	for (int i = 0; i < 3; i++)
	{
		v1[i] = edge2[i] - edge4[i];
		v2[i] = edge1[i] - edge4[i];
		v3[i] = edge2[i] - edge1[i];
	}

	normal_norm = Vect(v1, v2);
	normal_norm = Normalize(normal_norm);

	cross = CrossLineAndPlane(cam, direction_norm, edge4, normal_norm);
	if (cross.empty()) return vector<double>();

	prpoint1 = ProjectPointOnLine(edge4, v2, cross);
	prpoint11 = ProjectPointOnLine(edge4, v2, edge2);
	prpoint2 = ProjectPointOnLine(edge1, v3, cross);
	prpoint22 = ProjectPointOnLine(edge1, v3, edge4);
	prpoint3 = ProjectPointOnLine(edge4, v1, cross);
	prpoint33 = ProjectPointOnLine(edge4, v1, edge1);

	for (int i = 0; i < 3; i++)
	{
		prvec1[i] = prpoint1[i] - cross[i];
		prvec2[i] = prpoint2[i] - cross[i];
		prvec3[i] = prpoint3[i] - cross[i];
		prvec11[i] = prpoint11[i] - edge2[i];
		prvec22[i] = prpoint22[i] - edge4[i];
		prvec33[i] = prpoint33[i] - edge1[i];
	}

	if (Scalar(prvec1, prvec11) >= -1e-5 && Scalar(prvec2, prvec22) >= -1e-5 && Scalar(prvec3, prvec33) >= -1e-5) return cross;

	return vector<double>();
}

vector<int> ShadowSphere(vector<Figure*> figures, vector<int> color, vector<double> Spoint, vector<double> light)
{
	for (auto it = figures.begin(); it < figures.end(); it++)
	{
		if ((*it)->dimension == 4)
		{
			vector<double> normkpov_norm(3), light_norm(3);
			for (int i = 0; i < 3; i++)
			{
				color[i] = (*it)->color[i];
				normkpov_norm[i] = Spoint[i] - (*it)->base_data[i];
				light_norm[i] = light[i] - Spoint[i];
			}
			normkpov_norm = Normalize(normkpov_norm);
			light_norm = Normalize(light_norm);
			double k = Scalar(normkpov_norm, light_norm);
			if (k <= 0) k = 0;
			for (int i = 0; i < 3; i++)
			{
				if (k < 0.992) color[i] = int(color[i] * k);
				if (k >= 0.992) color[i] = 255;
			}
			break;
		}
	}
	return color;
}

vector<double> CrossSphere(vector<double> cam, vector<double> direction_norm, vector<Figure*> figures, vector<double> settings)
{
	vector<double> center(3);
	double r = 0.0;

	for (auto it = figures.begin(); it < figures.end(); it++)
	{
		if ((*it)->dimension == 4)
		{
			for (int i = 0; i < 3; i++) center[i] = (*it)->base_data[i];
			r = (*it)->base_data[3];
			break;
		}
	}

	vector<double> tocam = { center[0] - cam[0], center[1] - cam[1] , center[2] - cam[2] };
	vector<double> projection = ProjectPointOnLine(cam, direction_norm, center);
	double dist = DistanceBtwPoints(projection, center);

	if (Scalar(tocam, direction_norm) > 0)
	{
		if (abs(dist - r) <= 1e-5 && InZone(projection, settings)) return projection;

		if (dist > r) return vector<double>(0);
		vector<double> point1(3), point2(3);
		for (int i = 0; i < 3; i++)
		{
			point1[i] = projection[i] + direction_norm[i] * sqrt(r * r - dist * dist);
			point2[i] = projection[i] - direction_norm[i] * sqrt(r * r - dist * dist);
		}
		if (DistanceBtwPoints(cam, point1) < DistanceBtwPoints(cam, point2) && InZone(point1, settings)) return point1;

		if (InZone(point2, settings)) return point2;
	}

	if (DistanceBtwPoints(cam, center) > r) return vector<double>(0);

	vector<double> tocam_norm;

	for (int i = 0; i < 3; i++)
	{
		tocam_norm[i] = cam[i] - projection[i];
	}

	vector<double> point(3);

	if (Scalar(tocam_norm, direction_norm) > 0)
	{
		for (int i = 0; i < 3; i++)
		{
			point[i] = projection[i] + direction_norm[i] * sqrt(r * r - dist * dist);
		}
		if (InZone(point, settings)) return point;
	}

	for (int i = 0; i < 3; i++)
	{
		point[i] = projection[i] + direction_norm[i] * (sqrt(r * r - dist * dist) + DistanceBtwPoints(cam, projection));
	}

	if (InZone(point, settings)) return point;

	return vector<double>();
}

vector<double> CrossBox(vector<double> cam, vector<double> direction_norm, vector<Figure*> figures, vector<double> settings, vector<double> light)
{
	vector<double> edge1(3), edge2(3), edge3(3), edge4(3), edge5(3), edge6(3), edge7(3), edge8(3), v(4),
		e1 = { 1,0,0 }, e2 = { 0,1,0 }, e3 = { 0,0,1 };
	double x, y, z, mindist, temp, kmax;
	vector<vector<double>> crosslist;


	for (auto it = figures.begin(); it < figures.end(); it++)
	{
		if ((*it)->dimension == 6)
		{
			for (int i = 0; i < 3; i++)
			{
				edge1[i] = (*it)->base_data[i];
				edge7[i] = (*it)->base_data[i + 3];
				v[i] = edge7[i] - edge1[i];
			}
			break;
		}
	}

	x = Scalar(v, e1);
	y = Scalar(v, e2);
	z = Scalar(v, e3);

	edge2 = { edge1[0], edge1[1] + y, edge1[2] };
	edge3 = { edge1[0] + x, edge1[1] + y, edge1[2] };
	edge4 = { edge1[0] + x, edge1[1], edge1[2] };
	edge5 = { edge1[0] , edge1[1], edge1[2] + z };
	edge6 = { edge1[0] , edge1[1] + y, edge1[2] + z };
	edge8 = { edge1[0] + x , edge1[1] , edge1[2] + z };

	v = CrossRectangle(cam, direction_norm, edge1, edge5, edge8, edge4);
	if (!v.empty() && InZone(v, settings))
	{
		v[3] = ShadowBox(v, light, edge1, edge5, edge8, edge6);
		crosslist.push_back(v);
	}
	v = CrossRectangle(cam, direction_norm, edge1, edge5, edge6, edge2);
	if (!v.empty() && InZone(v, settings))
	{
		v[3] = ShadowBox(v, light, edge1, edge5, edge6, edge7);
		crosslist.push_back(v);
	};
	v = CrossRectangle(cam, direction_norm, edge2, edge6, edge7, edge3);
	if (!v.empty() && InZone(v, settings))
	{
		v[3] = ShadowBox(v, light, edge2, edge6, edge7, edge5);
		crosslist.push_back(v);
	}
	v = CrossRectangle(cam, direction_norm, edge3, edge7, edge8, edge4);
	if (!v.empty() && InZone(v, settings))
	{
		v[3] = ShadowBox(v, light, edge3, edge7, edge8, edge5);
		crosslist.push_back(v);
	}
	v = CrossRectangle(cam, direction_norm, edge5, edge6, edge7, edge8);
	if (!v.empty() && InZone(v, settings))
	{
		v[3] = ShadowBox(v, light, edge5, edge6, edge7, edge1);
		crosslist.push_back(v);
	}
	v = CrossRectangle(cam, direction_norm, edge1, edge2, edge3, edge4);
	if (!v.empty() && InZone(v, settings))
	{
		v[3] = ShadowBox(v, light, edge1, edge2, edge3, edge5);
		crosslist.push_back(v);
	}

	if (!crosslist.empty())
	{
		v = (*crosslist.begin());
		mindist = DistanceBtwPoints(cam, v);
		kmax = v[3];
		for (auto it = crosslist.begin(); it < crosslist.end(); it++)
		{
			temp = DistanceBtwPoints(cam, *it);
			if ((*it)[3] > kmax) kmax = (*it)[3];
			if (temp < mindist)
			{
				mindist = temp;
				v = (*it);
			}
		}
		v[3] = kmax;
		return v;
	}

	return vector<double>(0);
}

double ShadowBox(vector<double> point, vector<double> light, vector<double> edge1, vector<double>edge5, vector<double>edge4, vector<double>edge8)
{
	vector<double> normkpov_norm(3), light_norm(3), v1(3), v2(3), e1(3), e2(3), proj(3), projvect(3), x(3);
	for (int i = 0; i < 3; i++)
	{
		e1[i] = edge5[i] - edge1[i];
		e2[i] = edge4[i] - edge1[i];
		light_norm[i] = light[i] - point[i];
	}
	light_norm = Normalize(light_norm);
	v1 = Vect(e1, e2);
	v2 = Vect(e2, e1);
	v1 = Normalize(v1);
	v2 = Normalize(v2);
	proj = ProjectPointOnPlane(edge8, edge1, v1);
	for (int i = 0; i < 3; i++)
	{
		projvect[i] = edge8[i] - proj[i];
	}

	if (Scalar(projvect, v1) > 0) normkpov_norm = v2;
	else normkpov_norm = v1;
	double k = Scalar(normkpov_norm, light_norm);

	return k;
}

vector<double> CrossRectangle(vector<double> cam, vector<double> direction_norm, vector<double> edge1, vector<double>edge5, vector<double>edge8, vector<double>edge4)
{
	vector<double> cross(4), v1(3), v2(3), normal_norm,
		prpoint1, prpoint2, prpoint3, prpoint4,
		prvec1(3), prvec2(3), prvec3(3), prvec4(3);

	for (int i = 0; i < 3; i++)
	{
		v1[i] = edge5[i] - edge1[i];
		v2[i] = edge4[i] - edge1[i];
	}

	normal_norm = Vect(v1, v2);
	normal_norm = Normalize(normal_norm);

	cross = CrossLineAndPlane(cam, direction_norm, edge1, normal_norm);
	if (cross.empty())  return vector<double>();
	prpoint1 = ProjectPointOnLine(edge4, v1, cross);
	prpoint2 = ProjectPointOnLine(edge1, v1, cross);
	prpoint3 = ProjectPointOnLine(edge1, v2, cross);
	prpoint4 = ProjectPointOnLine(edge5, v2, cross);
	for (int i = 0; i < 3; i++)
	{
		prvec1[i] = prpoint1[i] - cross[i];
		prvec2[i] = prpoint2[i] - cross[i];
		prvec3[i] = prpoint3[i] - cross[i];
		prvec4[i] = prpoint4[i] - cross[i];
	}

	if (Scalar(prvec1, prvec2) < 1e-5 && Scalar(prvec3, prvec4) < 1e-5) return cross;

	return vector<double>();
}

vector<double> CrossLineAndPlane(vector<double> cam, vector<double> direction_norm, vector<double> edge1, vector<double> normal_norm)
{
	vector<double> proj = ProjectPointOnPlane(cam, edge1, normal_norm), res(4), a(3);

	for (int i = 0; i < 3; i++)
	{
		normal_norm[i] = proj[i] - cam[i];
	}

	normal_norm = Normalize(normal_norm);

	double cosa = Scalar(normal_norm, direction_norm);

	if (abs(cosa - 0) < 1e-5) return vector<double>();

	for (int i = 0; i < 3; i++)
	{
		normal_norm[i] = normal_norm[i] * cosa;
		a[i] = direction_norm[i] - normal_norm[i];
	}

	a = Normalize(a);

	for (int i = 0; i < 3; i++)
	{
		res[i] = proj[i] + a[i] * DistanceBtwPoints(cam, proj) * tan(acos(cosa));
	}
	return res;
}

double DistanceBtwPoints(vector<double> point1, vector<double> point2)
{
	if (point1.size() < 3 || point2.size() < 3)
	{
		return 1.7e308;
	}
	double distance;
	distance = sqrt((point1[0] - point2[0]) * (point1[0] - point2[0]) +
		(point1[1] - point2[1]) * (point1[1] - point2[1]) +
		(point1[2] - point2[2]) * (point1[2] - point2[2]));
	return distance;
}

double Norma(vector<double> vector)
{
	double norma = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
	return norma;
}

vector<double> Normalize(vector<double> vector)
{
	double k = Norma(vector);
	if (k != 0)
	{
		for (int i = 0; i < 3; i++) vector[i] = vector[i] / k;
	}
	return vector;
}

double Scalar(vector<double> vector1, vector<double> vector2)
{
	double scalar = vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2];
	return scalar;
}

vector<double> Vect(vector<double>  normal_norm, vector<double> up_norm)
{
	if (normal_norm.size() != 3 || up_norm.size() != 3) return vector<double>();
	vector<double> vect = { normal_norm[1] * up_norm[2] - normal_norm[2] * up_norm[1],
		normal_norm[2] * up_norm[0] - normal_norm[0] * up_norm[2],
		normal_norm[0] * up_norm[1] - normal_norm[1] * up_norm[0] };
	return vect;
}

vector<double> ProjectPointOnLine(vector<double> startpoint, vector<double> direction, vector<double> point)
{
	vector<double> projection(3), topoint = { point[0] - startpoint[0], point[1] - startpoint[1], point[2] - startpoint[2] };
	for (int i = 0; i < 3; i++)
	{
		projection[i] = startpoint[i] + Normalize(direction)[i] * Scalar(topoint, direction) / Norma(direction);
	}
	return projection;
}

bool InZone(vector<double> point, vector<double> settings)
{
	int width = int(settings[12]), height = int(settings[13]);
	double distance = settings[9], limit = settings[10], alpha = settings[11],
		pi = 3.1415926535,
		metr = (2 * distance * tan(alpha * pi / 360)) / height;

	vector<double> cam = { settings[0], settings[1], settings[2] },
		normal_norm = { settings[3], settings[4], settings[5] },
		up_norm = { settings[6] , settings[7] , settings[8] };

	normal_norm = Normalize(normal_norm);
	up_norm = Normalize(up_norm);

	vector<double> binorm_norm = Vect(up_norm, normal_norm);

	vector<double> screen_edge(3);
	for (int k = 0; k < 3; k++)
	{
		screen_edge[k] = cam[k] + distance * normal_norm[k] + height * metr / 2 * up_norm[k] + width * metr / 2 * binorm_norm[k];
	}

	vector<double> limit_screen_edge(3);
	for (int k = 0; k < 3; k++)
	{
		limit_screen_edge[k] = cam[k] + limit * normal_norm[k] + limit / distance * height * metr / 2 * up_norm[k] +
			limit / distance * width * metr / 2 * binorm_norm[k];
	}

	vector<double> project1 = ProjectPointOnPlane(point, screen_edge, normal_norm),
		project2 = ProjectPointOnPlane(point, limit_screen_edge, normal_norm);
	vector<double> vect1(3), vect2(3);
	for (int i = 0; i < 3; i++)
	{
		vect1[i] = project1[i] - point[i];
		vect2[i] = project2[i] - point[i];
	}

	if (abs(vect2[0]) < 1e-5 && abs(vect2[1]) < 1e-5 && abs(vect2[2]) < 1e-5) return true;

	if (Scalar(vect1, normal_norm) > 0 || Scalar(vect2, normal_norm) < 0) return false;

	return true;
}

vector<double> ProjectPointOnPlane(vector<double> point, vector<double> start, vector<double> normal_norm)
{
	normal_norm = Normalize(normal_norm);
	vector<double> v(3), project(3);
	for (int i = 0; i < 3; i++) v[i] = point[i] - start[i];
	double dist = Scalar(v, normal_norm);
	for (int i = 0; i < 3; i++) project[i] = point[i] - dist * normal_norm[i];
	return project;
}