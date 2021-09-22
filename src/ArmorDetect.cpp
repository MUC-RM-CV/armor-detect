//
// Created by Henry on 2021/5/23.
//

#include "ArmorDetect.h"

void drawTetragon(cv::Mat &image, cv::Point2f *vertices, const cv::Scalar &color) {
    using cv::Scalar;
    int thickness = (int)ceil(2e-3 * image.cols);
    for (int j = 0; j < 4; j++) {
        cv::line(image, vertices[j], vertices[(j + 1) % 4], color, thickness);
    }
    int radius = (int)ceil(1e-2 * image.cols);
    // cv::circle(image, vertices[0], radius, Scalar(  0,   0, 255), -1); // red
    // cv::circle(image, vertices[1], radius, Scalar(  0, 255, 255), -1); // yellow
    // cv::circle(image, vertices[2], radius, Scalar(255,   0, 255), -1); // purple / violet
    // cv::circle(image, vertices[3], radius, Scalar(255,   0,   0), -1); // blue
}

std::tuple<cv::Mat, cv::Mat> ArmorDetect::findRedPreprocess(const cv::Mat& frame) const {
    std::vector<cv::Mat> channels; // BGR
    std::vector<cv::Mat> normalizedChannels;
    cv::split(frame, channels);
    // for (auto & channel : channels) {
    //     cv::Mat n;
    //     // cv::equalizeHist(channel, n);
    //     normalizedChannels.push_back(n);
    // }
    // cv::Mat normalized;
    // cv::merge(normalizedChannels, normalized);

    cv::Mat highlightImg;
    cv::threshold(channels[1], highlightImg, this->params.highlightThresholdForRed, 255, cv::THRESH_BINARY);

    cv::Mat colorImg = channels[2] - channels[0];
    cv::threshold(colorImg, colorImg, this->params.colorThresholdForRed, 255, cv::THRESH_BINARY);
    cv::erode(colorImg, colorImg, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

    // cv::imshow("highlight", highlightImg);
    // cv::imshow("colorImg", colorImg);

    return std::make_tuple(highlightImg, colorImg);
}

std::tuple<cv::Mat, cv::Mat> ArmorDetect::findBluePreprocess(const cv::Mat& frame) const {
    std::vector<cv::Mat> channels; // BGR
    std::vector<cv::Mat> normalizedChannels;
    cv::split(frame, channels);
    // for (auto & channel : channels) {
    //     cv::Mat n;
    //     // cv::equalizeHist(channel, n);
    //     normalizedChannels.push_back(n);
    // }
    // cv::Mat normalized;
    // cv::merge(normalizedChannels, normalized);

    cv::Mat highlightImg;
    cv::threshold(channels[1], highlightImg, this->params.colorThresholdForBlue, 255, cv::THRESH_BINARY);

    cv::Mat colorImg = channels[0] - channels[2];
    cv::threshold(colorImg, colorImg, this->params.colorThresholdForBlue, 255, cv::THRESH_BINARY);

    // cv::imshow("highlight", highlightImg);
    // cv::imshow("colorImg", colorImg);

    return std::make_tuple(highlightImg, colorImg);
}

std::vector<LightBar> ArmorDetect::findLightBars(const cv::Mat& highlight, const cv::Mat& color) const {

    ContoursVector colorContours;
    ContoursVector highlightContours;

    cv::findContours(color, colorContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(highlight, highlightContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::RotatedRect> colorBoundingRRects;

    for (const auto& c : colorContours) {
        colorBoundingRRects.push_back(cv::minAreaRect(c));
    }

    std::vector<LightBar> lightBars;

    auto isInside = [](const cv::Rect& a, const cv::Rect& b) { return (a == (a & b)); };

    // ---
    // cv::Mat show(cv::Size(848, 586), CV_8UC3);
    // ---


    for (auto&& h : highlightContours) {

        auto box = cv::minAreaRect(h);
        auto boxArea = box.size.width * box.size.height;



        if (boxArea < this->params.minLightBarArea) continue;

        auto width = box.size.width;
        auto height = box.size.height;
        auto angle = box.angle;

        // -----
        // std::cout << "height: " << height << " width: " << width << " " << angle << "\n";
        // cv::Point2f pts[4];
        // box.points(pts);
        // drawTetragon(show, pts, cv::Scalar(0,255,0));
        // cv::imshow("show", show);
        // cv::waitKey(0);
        // -----

        if (height > width)  { // light bar like: '//'
            angle = 90 - angle;
        } else { // light bar like: '\\'
            std::swap(height, width);
            angle = 180 - angle;
        }

        // height > width

        auto ratio = height / width;

        if (ratio > Armor::maxLightBarLengthWidthRatio) continue;
        if (angle < Armor::lightBarMinAngle || angle > Armor::lightBarMaxAngle) continue;


        bool inside = false;

        for (const auto& r : colorBoundingRRects) {
            std::vector<cv::Point2f> intersectingRegion;
            if (cv::rotatedRectangleIntersection(r, box, intersectingRegion) != cv::INTERSECT_NONE) {
                inside = true;
                break;
            }
        }

        if (inside) {
            lightBars.emplace_back(box, angle, height, width);
        }
    }

    return lightBars;
}

std::vector<Armor> ArmorDetect::lightBarsPairing(const std::vector<LightBar> &lightBars) const {
    if (lightBars.size() < 2) return std::vector<Armor>();

    std::vector<Armor> armors;

    for (size_t i = 0; i < lightBars.size() - 1; i++) {
        for (size_t j = i + 1; j < lightBars.size(); j++) {
            auto maxBarLength = std::max(lightBars[i].length, lightBars[j].length);

            if (abs(lightBars[i].angle - lightBars[j].angle) > Armor::maxAngleBetweenLightBars) continue; // 两灯条夹角的差
            if (abs(lightBars[i].length - lightBars[j].length) / maxBarLength > 0.5) continue; // 两灯条长度的差
            if (abs(lightBars[i].center.x - lightBars[j].center.x) / maxBarLength > Armor::maxArmorWidthToLightBarLength) continue;
            if (abs(lightBars[i].center.y - lightBars[j].center.y) > this->params.lightBarsCenterMaxDiffY) continue;

            armors.emplace_back(lightBars[i], lightBars[j]);
        }
    }
    return armors;
}

Armor ArmorDetect::choice(const std::vector<Armor>& armors) {
    return armors[0];
}
