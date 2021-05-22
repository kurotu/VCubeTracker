#include <iostream>
#include <opencv2/aruco.hpp>
#include <opencv2/core/affine.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "common.h"

#define PRINT(var) (std::cout << #var << ": " << var << std::endl)

constexpr auto CUBE_LENGTH_PX = 360;
constexpr auto MARKER_LENGTH_PX = 280;
constexpr auto MARKER_SEPARATION_PX = 40;
constexpr auto NUMBER_OF_CUBES = 3;
constexpr auto DICTIONARY_ID =
    cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_APRILTAG_25h9;

bool endsWith(const std::string& str, const std::string& end) {
  return str.find(end, str.size() - end.size()) != std::string::npos;
}

std::tuple<int, int, int> chooseKeyAndSub(const std::vector<int>& results) {
  constexpr auto err = std::make_tuple(-1, -1, -1);
  if (std::count_if(results.begin(), results.end(),
                    [](auto r) { return r > 0; }) != 2) {
    return err;
  }
  for (auto i = 0; i < NUMBER_OF_CUBES; i++) {
    for (auto j = 0; j < 5; j++) {
      const auto keyIndex = i * 6;
      const auto subIndex = keyIndex + j + 1;
      if (results[keyIndex] > 0 && results[subIndex] > 0) {
        return std::make_tuple(i, keyIndex, subIndex);
      }
    }
  }
  return err;
}

int main(int argc, char** argv) {
  constexpr auto keys =
      "{help h usage ? |            | Print this message }"
      "{cl cube-length |<none>      | Cube length [m] }"
      "{ci camera-id   |0           | Camera ID to capture }"
      "{camera         |camera.yaml | Camera parameter file }"
      "{ps-eye         |false       | Use PlayStation Eye }"
      "{@outfile       |<none>      | Output file (*.yaml) }";

  cv::CommandLineParser parser(argc, argv, keys);
  if (parser.has("help")) {
    parser.printMessage();
    return 0;
  }
  const auto cubeLength = parser.get<double>("cl");
  const auto cameraID = parser.get<int>("ci");
  const auto cameraFile = parser.get<std::string>("camera");
  const auto usePSEye = parser.get<bool>("ps-eye");
  const auto outFile = parser.get<std::string>("@outfile");
  if (!parser.check()) {
    parser.printErrors();
    return 0;
  }
  if (!endsWith(outFile, ".yaml")) {
    std::cout << "outfile must end with \".yaml\". (" << outFile << ")"
              << std::endl;
    return 0;
  }
  const auto markerLength = cubeLength * static_cast<double>(MARKER_LENGTH_PX) /
                            static_cast<double>(CUBE_LENGTH_PX);
  const auto markerSeparation = cubeLength *
                                static_cast<double>(MARKER_SEPARATION_PX) /
                                static_cast<double>(CUBE_LENGTH_PX);

  cv::FileStorage cameraFS(cameraFile, cv::FileStorage::READ);
  if (!cameraFS.isOpened()) {
    std::cout << "Failed to open " << cameraFile << std::endl;
    return 1;
  }
  const int imageWidth = cameraFS["image_width"];
  const int imageHeight = cameraFS["image_height"];
  const auto cameraMatrix = cameraFS["camera_matrix"].mat();
  const auto distCoeffs = cameraFS["distortion_coefficients"].mat();
  cameraFS.release();

  std::unique_ptr<CameraCaptureBase> camera;
  auto fps = 0.0;
  if (usePSEye) {
    camera = decltype(camera)(new HighguiCapture(cameraID));
    fps = 60.0;
  } else {
    camera = decltype(camera)(new CameraCapture(cameraID));
  }
  if (camera->Open(imageWidth, imageHeight, fps) != 0) {
    std::cout << "Failed to open camera: " << cameraID << std::endl;
    return 1;
  }

  cv::Ptr<cv::aruco::Dictionary> dictionary =
      cv::aruco::getPredefinedDictionary(DICTIONARY_ID);
  std::vector<cv::Ptr<cv::aruco::Board>> gridBoards;
  for (int i = 0; i < 6 * NUMBER_OF_CUBES; i++) {
    constexpr auto x = 1;
    constexpr auto y = 1;
    constexpr auto rate = x * y;
    auto board = cv::aruco::GridBoard::create(
        x, y, markerLength, markerSeparation, dictionary, i * rate);
    for (auto& corners : board->objPoints) {
      for (auto& p : corners) {
        auto diff = markerLength / 2.0;
        p = p - cv::Point3f(diff, diff, 0.0);
      }
    }
    gridBoards.push_back(board);
  }

  // std::vector<cv::Affine3d> affinesLLeg(5), affinesRLeg(5), affinesHips(5);

  constexpr auto windowName = "camera";
  const auto div = imageWidth >= 1920 ? 2 : 1;

  auto windowWidth = imageWidth / div;
  auto windowHeight = imageHeight / div;
  cv::namedWindow(windowName);
  auto windowSize = cv::Size(windowWidth, windowHeight);
  cv::resizeWindow(windowName, windowSize);

  cv::Mat image(imageHeight, imageWidth, CV_8UC3);
  std::vector<std::vector<cv::Affine3d>> affines(NUMBER_OF_CUBES,
                                                 std::vector<cv::Affine3d>(5));
  std::vector<std::vector<bool>> estimated(NUMBER_OF_CUBES,
                                           std::vector<bool>(5, false));
  for (auto& v : estimated) {
    v[0] = true;
  }
  auto detectorParams = cv::aruco::DetectorParameters::create();
  detectorParams->cornerRefinementMethod = cv::aruco::CORNER_REFINE_APRILTAG;
  while (true) {
    camera->getImage(image);
    cv::Mat image2 = image.clone();
    cv::Mat invert;
    cv::bitwise_not(image, invert);

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners, rejectedCorners;
    cv::aruco::detectMarkers(invert, dictionary, corners, ids, detectorParams,
                             cv::noArray(), cameraMatrix, distCoeffs);
    refineDetectedMarkers(invert, gridBoards.begin(), gridBoards.end(), corners,
                          ids, rejectedCorners, cameraMatrix, distCoeffs);

    std::vector<cv::Vec3d> rvecs, tvecs;
    std::vector<int> results;
    if (ids.size() > 0) {
      cv::aruco::drawDetectedMarkers(image, corners, ids);
      estimatePoseBoards(corners, ids, gridBoards.begin(), gridBoards.end(),
                         cameraMatrix, distCoeffs, rvecs, tvecs, results);

      for (auto i = 0; i < results.size(); i++) {
        if (results[i] > 0) {
          cv::aruco::drawAxis(image, cameraMatrix, distCoeffs, rvecs[i],
                              tvecs[i], 0.025f);
        }
      }
    }
    const auto done =
        std::count_if(estimated.begin(), estimated.end(), [](auto v) {
          return std::count(v.begin(), v.end(), true) == v.size();
        }) == estimated.size();
    if (ids.size() > 0 && done) {
      std::vector<cv::Ptr<cv::aruco::Board>> cubes;
      for (auto i = 0; i < NUMBER_OF_CUBES; i++) {
        const auto keyIndex = i * 6;
        const auto subIndex = i * 6 + 5;
        const auto cube =
            createBoardCube(decltype(gridBoards)(gridBoards.begin() + keyIndex,
                                                 gridBoards.begin() + subIndex),
                            affines[i]);
        cubes.push_back(cube);
      }
      std::vector<cv::Vec3d> crvecs, ctvecs;
      std::vector<int> cresults;
      estimatePoseBoards(corners, ids, cubes.begin(), cubes.end(), cameraMatrix,
                         distCoeffs, crvecs, ctvecs, cresults);
      for (auto i = 0; i < cresults.size(); i++) {
        if (cresults[i] > 0) {
          cv::aruco::drawAxis(image2, cameraMatrix, distCoeffs, crvecs[i],
                              ctvecs[i], 0.05f);
        }
      }
    }
    cv::Mat scale;
    cv::resize(image, scale, windowSize);
    cv::putText(scale, "Press 'c' to capture", cv::Point(10, 30),
                cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255));
    cv::imshow("camera", scale);

    /*
    if (done) {
      cv::Mat scale2;
      cv::resize(image2, scale2, windowSize);
      cv::imshow("cube", scale2);
    }
    */
    auto key = cv::waitKey(1);
    if (key == 'c' && ids.size() > 0) {
      const auto idxs = chooseKeyAndSub(results);
      const auto cubeIndex = std::get<0>(idxs);
      const auto keyIndex = std::get<1>(idxs);
      const auto subIndex = std::get<2>(idxs);
      if (cubeIndex >= 0) {
        auto key = cv::Affine3d(rvecs[keyIndex], tvecs[keyIndex]);
        auto sub = cv::Affine3d(rvecs[subIndex], tvecs[subIndex]);
        auto base =
            key * cv::Affine3d(cv::Vec3d(), cv::Vec3d(0, 0, -cubeLength));
        auto i = subIndex - keyIndex;
        affines[cubeIndex][0] = base.inv() * key;
        affines[cubeIndex][i] = base.inv() * sub;
        estimated[cubeIndex][i] = true;
      }
      std::cout << cubeIndex << ", " << keyIndex << ", " << subIndex
                << std::endl;
    }
    if (key == 27) {  // esc
      // check all true

      if (done) {
        std::cout << "Calibration finished." << std::endl;
        break;
      } else {
        std::cout << "Calibration doesn't finish yet." << std::endl;
      }
    }
  }
  std::vector<cv::Ptr<cv::aruco::Board>> cubes;
  for (auto i = 0; i < NUMBER_OF_CUBES; i++) {
    const auto keyIndex = i * 6;
    const auto subIndex = i * 6 + 5;
    const auto cube =
        createBoardCube(decltype(gridBoards)(gridBoards.begin() + keyIndex,
                                             gridBoards.begin() + subIndex),
                        affines[i]);
    cubes.push_back(cube);
  }
  cv::FileStorage markerFS;
  markerFS.open(outFile, cv::FileStorage::WRITE);
  auto re = markerFS.isOpened();
  markerFS << "dictionary" << DICTIONARY_ID;
  markerFS << "boards"
           << "[";
  for (auto& c : cubes) {
    markerFS << c;
  }
  markerFS << "]";
  return 0;
}
