
#include "stdafx.h"

#include "markupplate.h"

Rect MarkupPlate::s_rects[FRM + 1];

#ifdef PICLOG

#if _WIN32 || _WIN64
#include <direct.h>
#define mkdir(p) _mkdir(p)
#else
#include <sys/types.h>
#include <sys/stat.h>
#define mkdir(p) mkdir(p, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

vector<string> PicLog::s_stack;
string PicLog::s_path;

void PicLog::savepath()
{
	s_stack.push_back(s_path);
}

void PicLog::makepath()
{
	mkdir(s_path.c_str());
}

void PicLog::restorepath()
{
	s_path = s_stack[s_stack.size() - 1];
	s_stack.pop_back();
}

string PicLog::to_string(int i)
{
	char buf[64];
	sprintf(buf, "%i", i);
	return string(buf);
}

string PicLog::to_string(double d)
{
	char buf[64];
	sprintf(buf, "%.2f", d);
	return string(buf);
}
#endif

bool MarkupRegion::find(const Size &size, const Rect &region, vector<vector<Point> > &contours, MarkupRegion& markup)
{
	vector<MarkupRegion> hypotheses;

	for (int i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect(contours[i]);

		//
		//отсечение по размеру относительно size и region

		double k = 1.8;

		Rect extrgn = Rect(region.x + (int)round((double)region.width * 0.05), region.y + (int)(((double)region.height / 2.0) * (1.0 - k)), (int)((double)region.width * k),  (int)((double)region.height * k));
		
		if (extrgn.contains(rect.tl()) && extrgn.contains(rect.br()) && rect.width > 0.15 * region.width && rect.height > 0.4 * region.height && rect.height < 1.0 * region.height)
		{
			//
			//добавить новую гипотезу

			hypotheses.push_back(MarkupRegion());

			//
			//распределить по существующим гипотезам

			for (int j = 0; j < hypotheses.size(); j++)
			{
				hypotheses[j].test(rect, i);
			}
		}
	}

	//
	//выбрать лучшую гипотезу

	int maxi = -1;
	double maxnum;

	for (int i = 0; i < hypotheses.size(); i++)
	{
		double num = hypotheses[i].num();

		if (maxi == -1 || num > maxnum)
		{
			maxi = i;
			maxnum = num;
		}
	}

	if (maxi == -1 || maxnum < 2)
	{
		return false;
	}

	if (!hypotheses[maxi].build(size, region, contours))
	{
		return false;
	}

	markup = hypotheses[maxi];

	return true;
}

Rect MarkupRegion::formingleft(const Rect &rect, double k, int width)
{
	Rect r;
	r.x = rect.x - width;
	r.width = width;
	r.y = rect.y + (int)((1.0 - k) * (double)rect.height / 2.0);
	r.height = (int)((double)rect.height * k);
	return r;
}

Rect MarkupRegion::formingright(const Rect &rect, double k, int width)
{
	Rect r;
	r.x = rect.x + rect.width;
	r.width = width;
	r.y = rect.y + (int)((1.0 - k) * (double)rect.height / 2.0);
	r.height = (int)((double)rect.height * k);
	return r;
}

MarkupRegion::MarkupRegion()
{
	clear();
}

MarkupRegion::MarkupRegion(const MarkupRegion& markup)
{
	for (int i = L0; i < 3; i++)
	{
		rects[i] = markup.rects[i];
		contouridxs[i] = markup.contouridxs[i];
	}
}

MarkupRegion& MarkupRegion::operator=(const MarkupRegion& markup)
{
	for (int i = L0; i < 3; i++)
	{
		rects[i] = markup.rects[i];
		contouridxs[i] = markup.contouridxs[i];
	}
	return *this;
}

MarkupRegion::~MarkupRegion()
{
}

void MarkupRegion::clear()
{
	for (int i = L0; i < 3; i++)
	{
		rects[i].x = 0;
		rects[i].y = 0;
		rects[i].width = 0;
		rects[i].height = 0;
		contouridxs[i] = -1;
	}
}

bool MarkupRegion::insert(int i, const Rect &rect, int contouridx)
{
	for (int k = 2; k > i; k--)
	{
		rects[k] = rects[k - 1];
		contouridxs[k] = contouridxs[k - 1];
	}

	rects[i] = rect;
	contouridxs[i] = contouridx;

	return true;
}

bool MarkupRegion::test(Rect &rect, int contouridx)
{
	//
	// габаритная ширина для контроля дистанции

	int X = -1;
	int W;
	int N = 0;
	int Sw = 0;

	for (int i = 0; i < 3; i++)
	{
		if (contouridxs[i] != -1)
		{
			if (X == -1)
			{
				X = rects[i].x;
			}

			W = rects[i].x + rects[i].width - X;

			Sw += rects[i].width;

			N++;
		}
	}

	int iend = 0;

	if (N != 0)
	{
		int Lim = 4 * (int)((double)Sw / (double)N);

		//
		//определение возможности, определение места и вставка

		for (int i = 0; i < 3; i++)
		{
			if (contouridxs[i] != -1)
			{
				//
				//совпадение по x todo контроль пропорций и поглощение бОльшим

				if (MarkupPlate::testreleps(0.02, rect.x, rects[i].x))
				{
					return false;
				}

				//
				//совпадение по y

				if (!MarkupPlate::testreleps(0.1, rect.y + rect.height, rects[i].y + rects[i].height))
				{
					return false;
				}

				//
				//сходство ширины

				if (!MarkupPlate::testreleps(0.45, (double)rect.width, (double)rects[i].width))
				{
					return false;
				}

				//
				//сходство высоты

				if (!MarkupPlate::testreleps(0.25, (double)rect.height, (double)rects[i].height))
				{
					return false;
				}

				//
				//контроль дистанции

				int d = rect.x - (X + (int)((double)W / 2.0));
				d = d < 0 ? -d : d;

				if (d > Lim)
				{
					return false;
				}

				//
				//вставка в место

				if (rect.x < rects[i].x)
				{
					insert(i, rect, contouridx);
					return true;
				}

				iend = i + 1;
			}
		}
	}

	//
	//вставка последним

	if (iend < 3)
	{
		insert(iend, rect, contouridx);
	}

	return true;
}

bool MarkupRegion::build(const Size &size, const Rect &region, vector<vector<Point> > &contours)
{
	double eps1 = 0.35;
	double eps2 = 0.2;
	double Kext = 1.1;

	if (contouridxs[2] != -1)
	{
		//три цифры найдены, всё ок
	}
	else if (contouridxs[1] != -1)
	{
		//две цифры найдены

		if (rects[0].x - rects[0].width > region.x/*|| MarkupPlate::testreleps(eps1, rects[0].x - region.x, rects[0].width)*/) //?есть ли место для цифры слева
		{
			//сдвигаем эти две вправо

			rects[2] = rects[1];
			contouridxs[2] = contouridxs[1];
		
			rects[1] = rects[0];
			contouridxs[1] = contouridxs[0];

			rects[0] = formingleft(rects[1], Kext, (int)((double)Kext * rects[1].width));
		}
		else if (rects[1].x - rects[0].width > rects[0].x + rects[0].width || MarkupPlate::testreleps(eps1, rects[1].x - (rects[0].x + rects[0].width), rects[0].width)) //есть ли место для цифры между ними
		{
			//сдвигаем одну вправо
			
			rects[2] = rects[1];
			contouridxs[2] = contouridxs[1];

			rects[1] = formingleft(rects[2], Kext, rects[2].x - (rects[0].x + rects[0].width));
		}
		else
		{
			rects[2] = formingright(rects[1], Kext, (int)((double)Kext * rects[1].width));
		}
	}
	else if (contouridxs[0] != -1)
	{
		//одна цифра найдена

		if (rects[0].x - 2 * rects[0].width > region.x || MarkupPlate::testreleps(eps2, rects[0].x - region.x, 2 * rects[0].width)) //есть ли место для двух цифр слева
		{
			rects[2] = rects[0];
			contouridxs[2] = contouridxs[0];

			rects[1] = formingleft(rects[2], Kext, (int)((double)Kext * rects[2].width));

			rects[0] = formingleft(rects[1], Kext, (int)((double)Kext * rects[2].width));
		}
		else if (rects[0].x - rects[0].width > region.x || MarkupPlate::testreleps(eps1, rects[0].x - region.x, rects[0].width)) //есть ли место для одной цифры слева
		{
			rects[1] = rects[0];
			contouridxs[1] = contouridxs[0];

			rects[0] = formingleft(rects[1], Kext, (int)((double)Kext * rects[1].width));

			rects[2] = formingright(rects[1], Kext, (int)((double)Kext * rects[1].width));
		}
		else
		{
			rects[1] = formingright(rects[0], Kext, (int)((double)Kext * rects[0].width));

			rects[2] = formingright(rects[1], Kext, (int)((double)Kext * rects[0].width));
		}
	}
	else
	{
		//ни одной цифры не найдено
		return false;
	}

	return true;
}

int MarkupRegion::num()
{
	int n = 0;

	for (int i = 0; i < 3; i++)
	{
		if (contouridxs[i] != -1)
		{
			n++;
		}
	}

	return n;
}

bool MarkupPlate::init()
{
	//
	//эталон

	s_rects[L0] = Rect(8 + 0, 4 + 9, 26 + 1, 36 + 1 - 9);
	s_rects[D1] = Rect(8 + 29, 4 + 0, 25 + 1, 36 + 1);
	s_rects[D2] = Rect(8 + 29 + 26, 4 + 0, 26 + 1, 36 + 1);
	s_rects[D3] = Rect(8 + 29 + 26 + 27, 4 + 0, 25 + 1, 36 + 1);
	s_rects[L1] = Rect(8 + 29 + 26 + 27 + 26, 4 + 9, 26 + 1, 36 + 1 - 9);
	s_rects[L2] = Rect(8 + 29 + 26 + 27 + 26 + 27, 4 + 9, 25 + 1, 36 + 1 - 9);
	s_rects[FRM] = Rect(0, 0, 229, 44);

	return true;
}

void MarkupPlate::uninit()
{
}

bool MarkupPlate::find(const Size &size, vector<vector<Point> > &contours, MarkupPlate& markup)
{
	vector<MarkupPlate> hypotheses;

	for (int i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect(contours[i]);

		//
		//отсечение по размеру относительно size
		
		if (testrect(size, rect))
		{
			//
			//добавить новую гипотезу

			hypotheses.push_back(MarkupPlate());

			//
			//распределить по существующим гипотезам

			for (int j = 0; j < hypotheses.size(); j++)
			{
				hypotheses[j].test(rect, i);
			}
		}
	}

	//
	//выбрать лучшую гипотезу

	int mini = -1;
	double mindeviation;

	for (int i = 0; i < hypotheses.size(); i++)
	{
		double deviation = hypotheses[i].deviation();

		if (mini == -1 || deviation < mindeviation)
		{
			mini = i;
			mindeviation = deviation;
		}
	}

	if (mini == -1 || mindeviation > MAXDEVIATION)
	{
		return false;
	}

	if (!hypotheses[mini].build(size, contours))
	{
		return false;
	}

	markup = hypotheses[mini];

	return true;
}

void MarkupPlate::drawframe(Mat &image, const Rect &rect, Scalar &color, bool crossed)
{
	rectangle(image, rect, color);

	if (crossed)
	{
		line(image, Point(rect.x, rect.y), Point(rect.x + rect.width - 1, rect.y + rect.height - 1), color);
		line(image, Point(rect.x, rect.y + rect.height -1), Point(rect.x + rect.width - 1, rect.y), color);
	}
}

void MarkupPlate::draw(Mat &image, Rect* rects)
{
	Point offset = Point(0, 0);
	draw(image, rects, offset);
}

void MarkupPlate::draw(Mat &image, Rect* rects, Point &offset)
{
	bool crossed = false;

	Scalar red = Scalar(0, 0, 255);
	Scalar green = Scalar(0, 255, 0);

	drawframe(image, rects[L0] + offset, red, crossed);
	drawframe(image, rects[D1] + offset, green, crossed);
	drawframe(image, rects[D2] + offset, red, crossed);
	drawframe(image, rects[D3] + offset, green, crossed);
	drawframe(image, rects[L1] + offset, red, crossed);
	drawframe(image, rects[L2] + offset, green, crossed);
	drawframe(image, rects[R1] + offset, red, crossed);
	drawframe(image, rects[R2] + offset, green, crossed);
	drawframe(image, rects[R3] + offset, red, crossed);
	drawframe(image, rects[RGN] + offset, green, false);
	drawframe(image, rects[FRM] + offset, red, false);
}

bool MarkupPlate::testrect(const Size &size, const Rect &rect)
{
	return 0.1 * (double)size.height < rect.height && rect.height < 0.95 * (double)size.height && 0.01 * (double)size.width < rect.width && rect.width < 0.2 * (double)size.width;
}

bool MarkupPlate::testreleps(double eps, double v1, double v2)
{
	double v = v1 / v2;

	if (v < 0)
	{
		return false;
	}

	v = v < 1 ? 1 - v : 1 - 1 / v;

	if (v > eps)
	{
		return false;
	}

	return true;
}

int MarkupPlate::compareheights(int height1, int height2)
{
	if (testreleps(0.1, (double)height1, (double)height2))
	{
		return EQ;
	}

	if (height1 < height2)
	{
		return LD_;
	}
	else
	{
		return DL;
	}
}

void MarkupPlate::improverect(vector<vector<Point> > &contours, const Size &size, Rect &rect)
{
	Rect improved(rect.x + (int)((double)rect.width / 2.0), rect.y + (int)((double)rect.height / 2.0), 0, 0);

	for (int k = 0; k < contours.size(); k++)
	{
		// отбрасываются только слишком маленькие
		if (0.1 * (double)size.height < rect.height && 0.01 * (double)size.width < rect.width)
		{
			Point min(-1, -1);
			Point max(-1, -1);

			for (int l = 0; l < contours[k].size(); l++)
			{
				Point p = contours[k][l];

				if (rect.contains(p))
				{
					min.x = (min.x == -1) ? p.x : (p.x < min.x ? min.x = p.x : min.x);
					max.x = (max.x == -1) ? p.x : (p.x > max.x ? max.x = p.x : max.x);
					min.y = (min.y == -1) ? p.y : (p.y < min.y ? min.y = p.y : min.y);
					max.y = (max.y == -1) ? p.y : (p.y > max.y ? max.y = p.y : max.y);
				}
			}

			if (min.x != -1)
			{
				improved |= Rect(min.x, min.y, max.x - min.x, max.y - min.y);
			}
		}
	}

	if (improved.area())
	{
		rect = improved;
	}
}

MarkupPlate::MarkupPlate()
{
	clear();
}

MarkupPlate::MarkupPlate(const MarkupPlate& markup)
{
	for (int i = L0; i < FRM + 1; i++)
	{
		rects[i] = markup.rects[i];
		contouridxs[i] = markup.contouridxs[i];
	}
}

MarkupPlate& MarkupPlate::operator=(const MarkupPlate& markup)
{
	for (int i = L0; i < FRM + 1; i++)
	{
		rects[i] = markup.rects[i];
		contouridxs[i] = markup.contouridxs[i];
	}
	return *this;
}

MarkupPlate::~MarkupPlate()
{
}

void MarkupPlate::clear()
{
	for (int i = L0; i < FRM + 1; i++)
	{
		rects[i].x = 0;
		rects[i].y = 0;
		rects[i].width = 0;
		rects[i].height = 0;
		contouridxs[i] = -1;
	}
}

bool MarkupPlate::insert(int i, const Rect &rect, int contouridx)
{
	for (int k = L2; k > i; k--)
	{
		rects[k] = rects[k - 1];
		contouridxs[k] = contouridxs[k - 1];
	}

	rects[i] = rect;
	contouridxs[i] = contouridx;

	return true;
}

bool MarkupPlate::test(Rect &rect, int contouridx)
{
	//
	// габаритная ширина для контроля дистанции

	int X = -1;
	int W;
	int N = 0;
	int Sw = 0;

	for (int i = L0; i < R1; i++)
	{
		if (contouridxs[i] != -1)
		{
			if (X == -1)
			{
				X = rects[i].x;
			}

			W = rects[i].x + rects[i].width - X;

			Sw += rects[i].width;

			N++;
		}
	}

	int iend = L0;

	if (N != 0)
	{
		int Lim = (R1 + 1) * (int)((double)Sw / (double)N);

		//
		//определение возможности, определение места и вставка

		for (int i = L0; i < R1; i++)
		{
			if (contouridxs[i] != -1)
			{
				//
				//совпадение по x todo контроль пропорций и поглощение бОльшим

				if (testreleps(0.02, rect.x, rects[i].x))
				{
					return false;
				}

				//
				//совпадение по y

				if (!testreleps(0.1, rect.y + rect.height, rects[i].y + rects[i].height))
				{
					return false;
				}

				//
				//сходство ширины

				if (!testreleps(0.25, (double)rect.width, (double)rects[i].width))
				{
					return false;
				}

				//
				//сходство высоты

				if (!testreleps(0.5, (double)rect.height, (double)rects[i].height))
				{
					return false;
				}

				//
				//контроль дистанции

				int d = rect.x - (X + (int)((double)W / 2.0));
				d = d < 0 ? -d : d;

				if (d > Lim)
				{
					return false;
				}

				//
				//вставка в место

				if (rect.x < rects[i].x)
				{
					insert(i, rect, contouridx);
					return true;
				}

				iend = i + 1;
			}
		}
	}

	//
	//вставка последним

	if (iend < R1)
	{
		insert(iend, rect, contouridx);
	}

	return true;
}

bool MarkupPlate::build(const Size &size, vector<vector<Point> > &contours)
{
	//массив должен быть непустой, L0 - всегда непустая (результатат сортировки по позициям всегда опирается на него)

	double eps1 = 0.35;
	double eps2 = 0.2;
	double Kext = 1.1;

	//
	//добавляем разрежения для потерянных

	int prev = -1;

	for (int i = L0; i < R1; i++)
	{
		if (contouridxs[i] != -1)
		{
			if (prev != -1)
			{
				int s = (int)round(((double)(rects[i].x - rects[prev].x)) / (double)rects[prev].width) - (i - prev);

				if (s < L2)
				{
					for (int j = 0; j < s; j++)
					{
						insert(i, Rect(), -1);
					}
				}
			}

			prev = i;
		}
	}

	//	
	//сдвигаем вправо для потерянных слева

	for (int i = D1; i < R1; i++)
	{
		if (contouridxs[i] != -1)
		{
			int cmp = compareheights(rects[L0].height, rects[i].height);

			int s = 0;

			switch (i)
			{
			case D1:
			{
				switch (cmp)
				{
				case EQ:
				{
					if (contouridxs[D2] != -1)
					{
						if (compareheights(rects[D1].height, rects[D2].height) == EQ)
						{
							s = 1;//3 didits
						}
						else
						{
							s = 2;
						}
						break;
					}
					else
					{
						s = 1;// возможны варианты, но т.к. потери большие, то нет смысла их анализировать
					}
				}
				break;

				//case LD_:
				//{
				//	s = 0;
				//}
				//break;

				case DL:
				{
					s = 3;
				}
				break;
				}
			}
			break;

			case D2:
			{
				switch (cmp)
				{
				case EQ:
				{
					s = 1;
				}
				break;

				//case LD_:
				//{
				//	s = 0;
				//}
				//break;

				case DL:
				{
					s = 2;
				}
				break;
				}
			}
			break;

			case D3:
			{
				switch (cmp)
				{
				//case EQ:
				//{
				//	//impossible
				//}
				//break;
				//
				//case LD_:
				//{
				//	s = 0;
				//}
				//break;

				case DL:
				{
					s = 1;
				}
				break;
				}
			}
			break;

			//case L1:
			//{
			//	switch (cmp)
			//	{
			//	case EQ:
			//	{
			//		s = 0;
			//	}
			//	break;
			//
			//	case LD_:
			//	{
			//		//impossible
			//	}
			//	break;
			//
			//	case DL:
			//	{
			//		//impossible
			//	}
			//	break;
			//	}
			//}
			//break;
			//
			//case L2:
			//{
			//	switch (cmp)
			//	{
			//	case EQ:
			//	{
			//		s = 0;
			//	}
			//	break;
			//
			//	case LD_:
			//	{
			//		//impossible
			//	}
			//	break;
			//
			//	case DL:
			//	{
			//		//impossible
			//	}
			//	break;
			//	}
			//}
			//break;
			}

			for (int j = 0; j < s; j++)
			{
				insert(L0, Rect(), -1);
			}

			break;
		}
	}

	//
	//габариты обнаруженных

	int i1 = -1;
	int i2 = -1;
	int i3 = -1;

	Rect rect1;
	Rect rect2;

	for (int i = L0; i < R1; i++)
	{
		if (contouridxs[i] != -1)
		{
			if (i1 == -1)
			{
				i1 = i;
			}

			i2 = i;

			if (ISD(i))
			{
				i3 = i;
			}
		}
	}

	rect1.x = rects[i1].x;
	rect1.y = rects[i1].y;
	rect1.width = rects[i2].x + rects[i2].width - rect1.x;
	rect1.height = rects[i2].y + rects[i2].height - rect1.y;

	if (i3 != -1)
	{
		rect1.y = rects[i3].y;
		rect1.height = rects[i3].height;
	}

	rect2.x = s_rects[i1].x;
	rect2.y = s_rects[i1].y;
	rect2.width = s_rects[i2].x + s_rects[i2].width - rect2.x;
	rect2.height = s_rects[i2].y + s_rects[i2].height - rect2.y;

	if (i3 != -1)
	{
		rect2.y = s_rects[i3].y;
		rect2.height = s_rects[i3].height;
	}

	//
	//формируем рамку

	double kx = (double)rect1.width / (double)rect2.width;
	double ky = (double)rect1.height / (double)rect2.height;

	double dx2 = (double)rect2.x + (double)rect2.width / 2.0 - (double)s_rects[FRM].x;
	double dy2 = (double)rect2.y + (double)rect2.height / 2.0 - (double)s_rects[FRM].y;

	rects[FRM].x = (int)((double)rect1.x + (double)rect1.width / 2.0 - dx2 * kx);
	rects[FRM].y = (int)((double)rect1.y + (double)rect1.height / 2.0 - dy2 * ky);
	rects[FRM].width = (int)((double)s_rects[FRM].width * kx);
	rects[FRM].height = (int)((double)s_rects[FRM].height * ky);

	rects[RGN].x = rects[FRM].x + (int)(0.765 * rects[FRM].width);
	rects[RGN].y = (int)(0.975 * rects[FRM].y);
	rects[RGN].width = int(0.31 * rects[FRM].width);
	rects[RGN].height = int(0.825 * rects[FRM].height);

	if (rects[FRM].x <= 0 || rects[FRM].y <= 0 || rects[FRM].width <= 0 || rects[FRM].height <= 0)
	{
		return false;
	}

	if (rects[FRM].x + rects[FRM].width > size.width || rects[FRM].y + rects[FRM].height > size.height)
	{
		return false;
	}

	//
	//формируем потерянные

	for (int i = L0; i < R1; i++)
	{
		if (contouridxs[i] == -1)
		{
			//
			//ищем левую границу

			if (i != L0 && rects[i - 1].width != 0)//если символ есть слева, включая уже определённые
			{
				rects[i].x = rects[i - 1].x + rects[i - 1].width;
			}
			else
			{
				//
				//ищем ближайший слева, включая уже определённые

				int j;

				for (j = i; j != -1; j--)
				{
					if (rects[j].width != 0)
					{
						break;
					}
				}

				if (j != -1)//если нашли
				{
					rects[i].x = rects[j].x + (int)round((s_rects[i].x - s_rects[j].x) * kx);
				}
				else
				{
					//по левой границе эталона
					rects[i].x = rects[FRM].x + (int)round((double)(s_rects[i].x - s_rects[FRM].x) * kx * 0.8);
				}
			}

			//
			//ищем правую границу

			if (i != L2 && rects[i + 1].width != 0)//если символ есть справа, включая уже определённые, хотя справа их не будет
			{
				rects[i].width = rects[i + 1].x - rects[i].x;
			}
			else
			{
				//по эталону
				rects[i].width = (int)round(s_rects[i].width * kx * Kext);
			}

			//
			//вертикали берём по эталону для цифры, но высоту расширяем относительно центра области

			rects[i].y = rects[FRM].y + (int)round((s_rects[D1].y - s_rects[FRM].y) * ky);
			rects[i].height = (int)round(s_rects[D1].height * ky);

			double c = (double)rects[i].y + (double)rects[i].height / 2.0;
			double h = (double)rects[i].height * Kext;

			rects[i].y = (int)round(c - h / 2);
			rects[i].height = (int)round(h);
		}
	}

	//
	//для потерянных обрезаем предварительно определённые рамки по контурам внутри них

	for (int i = L0; i < R1; i++)
	{
		if (contouridxs[i] == -1)
		{
			improverect(contours, size, rects[i]);
		}
	}

	//
	//формируем места цифр региона

	MarkupRegion markuprgn;

	if (MarkupRegion::find(size, rects[RGN], contours, markuprgn))
	{
		for (int i = 0; i < 3; i++)
		{
			rects[R1 + i] = markuprgn.rects[i];
			contouridxs[R1 + i] = markuprgn.contouridxs[i];
		}
	}
	else
	{
		//параметры требований отношения к рамке габаритов региона
		double rdfh = 0.6;
		double rdfw = 0.4;

		//
		//отыскиваем подходящий елемент в области региона

		double k = 1.05;

		Rect extrgn = Rect(rects[RGN].x + (int)round((double)rects[RGN].width * 0.05), rects[RGN].y + (int)(((double)rects[RGN].height / 2.0) * (1.0 - k)), (int)((double)rects[RGN].width * k),  (int)((double)rects[RGN].height * k));

		for (int i = 0; i < contours.size(); i++)
		{
			Rect rect = boundingRect(contours[i]);

			if (extrgn.contains(rect.tl()) && extrgn.contains(rect.br()) && rect.width > 0.25 * rects[RGN].width && rect.height > 0.6 * rects[RGN].height && rect.height < 1.0 * rects[RGN].height)
			{
				rects[R1] = rect;
				contouridxs[R1] = i;

				break;
			}
		}

		if (contouridxs[R1] != -1)
		{
			//одна цифра найдена

			if (rects[R1].x - 2 * rects[R1].width > rects[RGN].x || testreleps(eps2, rects[R1].x - rects[RGN].x, 2 * rects[R1].width)) //есть ли место для двух цифр слева
			{
				rects[R3] = rects[R1];
				contouridxs[R3] = contouridxs[R1];

				//формируем rect1 для R1
				Rect rect1(rects[RGN].x, rects[RGN].y, (int)((double)(rects[R3].x - rects[RGN].x) / 2.0), rects[RGN].height);

				rects[R1] = rect1;

				//вызываем для rect1 improverect, если результат нормальный - обновляем для R1
				improverect(contours, size, rect1);

				if (!(rects[RGN].height * rdfh > rect1.height || rects[RGN].width * rdfw < rect1.width))
				{
					rects[R1] = rect1;
				}

				//формируем rect2 для R2
				Rect rect2(rects[R1].x + rects[R1].width, rects[RGN].y, rects[R3].x - (rects[R1].x + rects[R1].width), rects[RGN].height);

				rects[R2] = rect2;

				//вызываем для rect2 improverect, если результат нормальный - обновляем для R2
				improverect(contours, size, rect2);

				if (!(rects[RGN].height * rdfh > rect2.height || rects[RGN].width * rdfw < rect2.width))
				{
					rects[R2] = rect2;
				}
			}
			else if (rects[R1].x - rects[R1].width > rects[RGN].x || testreleps(eps1, rects[R1].x - rects[RGN].x, rects[R1].width)) //есть ли место для одной цифры слева
			{
				rects[R2] = rects[R1];
				contouridxs[R2] = contouridxs[R1];

				//формируем rect1 для R1
				Rect rect1(rects[RGN].x, rects[RGN].y, rects[R2].x - rects[RGN].x, rects[RGN].height);

				rects[R1] = rect1;

				//вызываем для rect1 improverect, если результат нормальный - обновляем для R1
				improverect(contours, size, rect1);

				if (!(rects[RGN].height * rdfh > rect1.height || rects[RGN].width * rdfw < rect1.width))
				{
					rects[R1] = rect1;
				}

				//формируем rect3 для R3
				Rect rect3(rects[R2].x + rects[R2].width, rects[RGN].y, (int)((double)rects[R2].width * Kext), rects[RGN].height);

				//вызываем для rect3 improverect, если результат нормальный - используем для R3, иначе - отказываемся
				improverect(contours, size, rect3);

				if (!(rects[RGN].height * rdfh > rect3.height || rects[RGN].width * rdfw < rect3.width))
				{
					rects[R3] = rect3;
				}
			}
			else
			{
				//формируем rect2 для R2
				Rect rect2(rects[R1].x + rects[R1].width, rects[RGN].y, (int)((double)rects[R1].width * Kext), rects[RGN].height);

				rects[R2] = rect2;

				//вызываем для rect2 improverect, если результат нормальный - обновляем для R2
				improverect(contours, size, rect2);

				if (!(rects[RGN].height * rdfh > rect2.height || rects[RGN].width * rdfw < rect2.width))
				{
					rects[R2] = rect2;
				}

				//формируем rect3 для R3
				Rect rect3(rects[R2].x + rects[R2].width, rects[RGN].y, (int)((double)rects[R2].width * Kext), rects[RGN].height);

				//вызываем для rect3 improverect, если результат нормальный - используем для R3, иначе - отказываемся
				improverect(contours, size, rect3);

				if (!(rects[RGN].height * rdfh > rect3.height || rects[RGN].width * rdfw < rect3.width))
				{
					rects[R3] = rect3;
				}
			}
		}
		else
		{
			//ни одной цифры не найдено

			int w = (int)round((double)(rects[FRM].x +  rects[FRM].width - rects[RGN].x) / 2.0 * 0.85);

			//формируем rect1 для R1
			Rect rect1(rects[RGN].x, rects[RGN].y, w, rects[RGN].height);

			rects[R1] = rect1;

			//вызываем для rect1 improverect, если результат нормальный - обновляем для R1
			improverect(contours, size, rect1);

			if (!(rects[RGN].height * rdfh > rect1.height || rects[RGN].width * rdfw < rect1.width))
			{
				rects[R1] = rect1;
			}

			//формируем rect2 для R2
			Rect rect2(rects[R1].x + rects[R1].width, rects[RGN].y, w, rects[RGN].height);

			rects[R2] = rect2;

			//вызываем для rect2 improverect, если результат нормальный - обновляем для R2
			improverect(contours, size, rect2);

			if (!(rects[RGN].height * rdfh > rect2.height || rects[RGN].width * rdfw < rect2.width))
			{
				rects[R2] = rect2;
			}

			//формируем rect3 для R3
			Rect rect3(rects[R2].x + rects[R2].width, rects[RGN].y, w, rects[RGN].height);

			//вызываем для rect3 improverect, если результат нормальный - используем для R3, иначе - отказываемся
			improverect(contours, size, rect3);

			if (!(rects[RGN].height * rdfh > rect3.height || rects[RGN].width * rdfw < rect3.width))
			{
				rects[R3] = rect3;
			}
		}
	}

	return true;
}

double MarkupPlate::deviation()
{
	int missed = R1;
	double length = 0;

	double Fl = 40.1 / 55.0;
	double Fd = 41.3 / 72.0;

	double Kx;
	double Ky;

	if (rects[FRM].area())
	{
		Kx = (double)s_rects[FRM].width / (double)rects[FRM].width;
		Ky = (double)s_rects[FRM].height / (double)rects[FRM].height;
	}

	for (int i = L0; i < R1; i++)
	{
		if (contouridxs[i] != -1)
		{
			missed--;

			if (rects[FRM].area())
			{
				double x = WEIGHTX * (((double)rects[i].x + (double)rects[i].width / 2.0 - (double)rects[FRM].x) * Kx - ((double)s_rects[i].x + (double)s_rects[i].width / 2.0));

				double y = WEIGHTY * (((double)rects[i].y + (double)rects[i].height / 2.0 - (double)rects[FRM].y) * Ky - ((double)s_rects[i].y + (double)s_rects[i].height / 2.0));

				double w = WEIGHTW * ((ISL(i) ? Fl : Fd) * (double)rects[i].height * Ky - (double)rects[i].width * Kx);

				length += sqrt(x * x + y * y + w * w);
			}
		}
	}

	return missed != R1 ? WEIGHTMISSED * missed + length : INFDEVIATION;
}
