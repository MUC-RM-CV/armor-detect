//
// Created by Henry on 2021/5/23.
//

#ifndef MUC_CLEMENTINE_ARMOR_H
#define MUC_CLEMENTINE_ARMOR_H

#include <opencv2/opencv.hpp>
#include <utility>

struct LightBar {
    cv::RotatedRect box;
    float angle{};
    float length{};
    float thickness{};
    cv::Point2f center;

    LightBar() = default;

    LightBar(cv::RotatedRect b, float a, float l, float t)
            : box(std::move(b)), angle(a), length(l), thickness(t), center(b.center) {}
    LightBar(cv::RotatedRect && b, float a, float l, float t)
            : box(b), angle(a), length(l), thickness(t), center(b.center) {}
};

struct Armor {

public:
    constexpr static const float maxArmorWidthHeightRatio = 3.5;
    constexpr static const float maxArmorWidthToLightBarLength = 4;
    constexpr static const float armorHeightToLightBarLength = 2;
    constexpr static const float maxLightBarLengthWidthRatio = 10;
    constexpr static const float lightBarMinAngle = 60;
    constexpr static const float lightBarMaxAngle = 120;
    constexpr static const float maxAngleBetweenLightBars = 40;

    cv::Point2f center;
    float width{}; // 装甲板宽度取两个灯条中心的距离
    float height{}; // 装甲板高度取两个灯条的平均值乘上一个系数
    int id{};

    Armor() = default;

    Armor(LightBar l, LightBar r, int ID = 0) : leftBar(std::move(l)), rightBar(std::move(r)), id(ID) {
        center = (leftBar.center + rightBar.center) / 2;
        height = armorHeightToLightBarLength * (leftBar.length + rightBar.length) / 2;
        auto p = leftBar.center - rightBar.center;
        width = sqrt(p.x * p.x + p.y * p.y);
        if (leftBar.center.x > rightBar.center.x) std::swap(leftBar, rightBar);
    };

    Armor(LightBar && l, LightBar && r, int ID = 0) : leftBar(l), rightBar(r), id(ID) {
        center = (leftBar.center + rightBar.center) / 2;
        height = armorHeightToLightBarLength * (leftBar.length + rightBar.length) / 2;
        auto p = leftBar.center - rightBar.center;
        width = sqrt(p.x * p.x + p.y * p.y);
        if (leftBar.center.x > rightBar.center.x) std::swap(leftBar, rightBar);
    };

    void points(cv::Point2f pts[]) const {
        auto getVecFromCenter = [](const LightBar & bar) {
            auto radOfDeg = [](float deg) -> float { return deg / 180.0 * CV_PI; };
            return cv::Point2f(cos(radOfDeg(bar.angle)) * bar.length, -sin(radOfDeg(bar.angle) * bar.length)) / 2;
        };
        auto leftBarVec = getVecFromCenter(leftBar);
        auto rightBarVec = getVecFromCenter(rightBar);
        pts[0] = leftBar.center - leftBarVec; // bottom left;
        pts[1] = leftBar.center + leftBarVec; // top left;
        pts[2] = rightBar.center + rightBarVec; // top right;
        pts[3] = rightBar.center - rightBarVec; // bottom right;
    };

    void setID(int ID) { id = ID; };

    [[nodiscard]] auto boundingRect() const {
        auto delta = cv::Point2f(width * 0.9, height * 0.8);
        cv::Point2f tl = this->center - delta;
        cv::Point2f br = this->center + delta;
        return cv::Rect2f (tl, br);
    }

private:
    LightBar leftBar;
    LightBar rightBar;

};


#endif //MUC_CLEMENTINE_ARMOR_H
