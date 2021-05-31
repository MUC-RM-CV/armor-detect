//
// Created by Henry on 2021/5/23.
//

#ifndef MUC_CLEMENTINE_ARMORDETECT_H
#define MUC_CLEMENTINE_ARMORDETECT_H

#include <iostream>
#include <vector>
#include "Armor.h"

using ContoursVector = std::vector< std::vector< cv::Point > >;

struct ArmorDetectParams {
    int highlightThresholdForRed    = 200;
    int highlightThresholdForBlue   = 200;
    int colorThresholdForRed        = 30;
    int colorThresholdForBlue       = 30;
    float minLightBarArea           = 15;
    float lightBarsCenterMaxDiffY   = 20; // 0.05 * frame height
};

class ArmorDetect {
public:
    enum class Mode { FIND_RED, FIND_BLUE };

    explicit ArmorDetect(Mode m) : mode(m) {}

    std::vector<cv::Point2f> process(const cv::Mat& frame) {
        if (this->mode == Mode::FIND_RED) {
            auto [highlight, color] = findRedPreprocess(frame);
            lightBars = findLightBars(highlight, color);
            results = lightBarsPairing(lightBars);
        } else {
            auto [highlight, color] = findBluePreprocess(frame);
            lightBars = findLightBars(highlight, color);
            results = lightBarsPairing(lightBars);
        }

        if (results.empty()) {
            lastTarget = Armor();
            return std::vector<cv::Point2f>();
        } else {
            cv::Point2f pts[4];
            lastTarget = choice(results);
            lastTarget.points(pts);
            std::vector<cv::Point2f> aimArea = {pts[0], pts[1], pts[2], pts[3]};
            return aimArea;
        }
    }

    void setParams(ArmorDetectParams p) { params = p; }

    [[nodiscard]] const std::vector<Armor> & getResults() const { return results; }
    [[nodiscard]] const std::vector<LightBar> & getLightBars() const { return lightBars; }

private:
    std::vector<LightBar> lightBars;
    std::vector<Armor> results;

    Armor choice(const std::vector<Armor>& armors);

    Armor lastTarget;

    [[nodiscard]] std::tuple<cv::Mat, cv::Mat> findRedPreprocess(const cv::Mat& frame) const;
    [[nodiscard]] std::tuple<cv::Mat, cv::Mat> findBluePreprocess(const cv::Mat& frame) const;
    [[nodiscard]] std::vector<LightBar> findLightBars(const cv::Mat& highlight, const cv::Mat& color) const;
    [[nodiscard]] std::vector<Armor> lightBarsPairing(const std::vector<LightBar>& lightBars) const;

    Mode mode;

    ArmorDetectParams params;
};

void drawTetragon(cv::Mat &image, cv::Point2f *vertices, const cv::Scalar &color);

#endif //MUC_CLEMENTINE_ARMORDETECT_H
