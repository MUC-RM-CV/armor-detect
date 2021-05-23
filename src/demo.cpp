//
// Created by Henry on 2021/5/23.
//

#include "ArmorDetect.h"



int main(int argc, char* argv[]) {

    cv::namedWindow("frame");
    cv::namedWindow("show");

    cv::Mat frame;
    frame = cv::imread(cv::String(argv[1]));
    std::cout << frame.size();
    cv::imshow("frame", frame);
    cv::waitKey(0);

    cv::Mat show;
    frame.copyTo(show);

    ArmorDetect inst(ArmorDetect::Mode::FIND_BLUE);
    auto aimArea = inst.process(frame);

    if (aimArea.empty()) std::cout << "No target found.\n";
    else drawTetragon(show, aimArea.data(), cv::Scalar(255,255,255));
    cv::imshow("show", show);
    cv::waitKey(0);

    for (const auto & bar : inst.getLightBars()) {
        cv::Point2f pts[4];
        bar.box.points(pts);
        drawTetragon(show, pts, cv::Scalar(0,255,0));
    }

    cv::imshow("show", show);
    cv::waitKey(0);

    for (const auto& armor : inst.getResults()) {
        cv::Point2f pts[4];
        armor.points(pts);
        drawTetragon(show, pts, cv::Scalar(0,255,0));
    }

    cv::imshow("show", show);
    cv::waitKey(0);
}


