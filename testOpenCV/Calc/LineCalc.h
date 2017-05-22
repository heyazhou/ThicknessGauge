﻿#pragma once
#include <opencv2/core.hpp>
#include "../tg.h"

using namespace tg;

class LineCalc {
public:

#ifdef CV_VERSION
	
	bool intersection(cv::Point2f o1, cv::Point2f p1, cv::Point2f o2, cv::Point2f p2, cv::Point2f& r) const;

	bool intersection(const cv::Vec4f& border, cv::Vec4f& line, cv::Point2f& result) const;

	bool computeIntersectionPoints(cv::Vec4f horizontalLine, const cv::Vec4f & leftBorder, const cv::Vec4f & rightBorder, cv::Vec4f & output) const;

	static bool adjustMarkingRect(cv::Rect2f& markingRect, cv::Vec4f intersectionPoints, uint buffer);

	static bool adjustBaseLines(cv::Vec4f& baseLines, cv::Vec4f& intersectionPoints, uint buffer);

#endif




};

