#include <deque>
#include <iostream>
#include <mutex>
#include <numeric>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
// #include <openvr.h>
#include "ThreadPool.h"
#include "common.h"
#include "fake_tracker.h"
#include "openvr_util.h"

#define PRINT(var) (std::cout << #var << ": " << var << std::endl)

class TrackingConfiguration {
 public:
  bool usePSEye = false;
  bool useSteamVr = true;
  double fps = 0.0;
};

template <class C>
double getDurationInMillis(const C& start, const C& end) {
  return static_cast<double>(
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count() /
      1000.0);
}

double getMedian(std::vector<double> v) {
  if (v.size() % 2 == 0) {
    auto i = v.size() / 2 - 1;
    auto j = i + 1;
    std::nth_element(v.begin(), v.begin() + i, v.end());
    auto qi = *(v.begin() + i);
    std::nth_element(v.begin(), v.begin() + j, v.end());
    auto qj = *(v.begin() + j);
    return (qi + qj) / 2.0;
  } else {
    auto i = v.size() / 2;
    std::nth_element(v.begin(), v.begin() + i, v.end());
    return *(v.begin() + i);
  }
}

template <class Vec3dItr>
cv::Vec3d getMedian(const Vec3dItr& first, const Vec3dItr& last) {
  const auto size = std::distance(first, last);
  std::vector<double> vx(size), vy(size), vz(size);
  std::transform(first, last, vx.begin(), [](auto v) { return v[0]; });
  std::transform(first, last, vy.begin(), [](auto v) { return v[1]; });
  std::transform(first, last, vz.begin(), [](auto v) { return v[2]; });
  auto x = getMedian(vx);
  auto y = getMedian(vy);
  auto z = getMedian(vz);
  return {x, y, z};
}

cv::Vec3d getMedian(const std::deque<cv::Vec3d>& vecs) {
  return getMedian(vecs.begin(), vecs.end());
}
/*
TrackerData getMedian(const std::vector<TrackerData>& data) {
  std::vector<cv::Vec3d> tvecs(data.size());
  std::transform(data.begin(), data.end(), tvecs.begin(),
                 [](auto d) { return d.tvec; });
  std::vector<cv::Vec3d> rvecs(data.size());
  std::transform(data.begin(), data.end(), rvecs.begin(),
                 [](auto d) { return d.rvec; });
  auto t = getMedian(tvecs);
  auto r = getMedian(rvecs);
  return TrackerData{t, r};
}

TrackerData getMedian(const std::deque<TrackerData>& data) {
  std::vector<cv::Vec3d> tvecs(data.size());
  std::transform(data.begin(), data.end(), tvecs.begin(),
                 [](auto d) { return d.tvec; });
  std::vector<cv::Vec3d> rvecs(data.size());
  std::transform(data.begin(), data.end(), rvecs.begin(),
                 [](auto d) { return d.rvec; });
  auto t = getMedian(tvecs);
  auto r = getMedian(rvecs);
  return TrackerData{t, r};
}
*/

template <typename FloatType>
cv::Vec<FloatType, 3> getRvec(FloatType role, FloatType pitch, FloatType yaw) {
  auto affine = getAffine3d(role, pitch, yaw);
  return affine.rvec();
}

cv::Mat getRotatedPos(cv::Vec3d& t, cv::Vec3d& r) {
  cv::Mat rot;
  cv::Rodrigues(r, rot);
  return rot * t;
}

std::pair<cv::Vec3d, Eigen::Quaterniond> getTrackerData(
    const cv::Affine3d& cameraAffine, cv::Vec3d& tvec, cv::Vec3d& rvec) {
  auto pose = cameraAffine * cv::Affine3d(rvec, tvec);
  auto pos = pose.translation();
  auto quat = rvecToQuaternion(pose.rvec());
  auto data = std::make_pair(pos, quat);
  return std::move(data);
}

std::pair<std::array<float, 3>, std::array<float, 4>> getTrackerDataZMQ(
    const cv::Affine3d& cameraAffine, cv::Vec3d& tvec, cv::Vec3d& rvec) {
  auto pose = getTrackerData(cameraAffine, tvec, rvec);
  auto& pos = pose.first;
  auto& quat = pose.second;
  std::array<float, 4> q = {quat.x(), quat.y(), quat.z(), quat.w()};
  std::array<float, 3> p = {pos[0], pos[1], pos[2]};
  auto data = std::make_pair(p, q);
  return std::move(data);
}

UdpTransmitSocket transmitSocket(IpEndpointName("127.0.0.1", 39570));
std::mutex tracker_mtx;
static vr::IVRSystem* m_VRSystem;
Eigen::Vector3d hmdPos;
Eigen::Quaterniond hmdQuat;
bool shouldExit = false;
/*
template <class TrackerDataItr>
TrackerData getAverageT(const TrackerDataItr& first,
                        const TrackerDataItr& last) {
  auto tsum = std::accumulate(first, last, cv::Vec3d(0.0, 0.0, 0.0),
                              [](auto& acc, auto& i) { return acc + i.tvec; });
  auto rsum = std::accumulate(first, last, cv::Vec3d(0.0, 0.0, 0.0),
                              [](auto& acc, auto& i) { return acc + i.rvec; });
  auto size = static_cast<double>(std::distance(first, last));
  return TrackerData{tsum / size, rsum / size};
}
*/
// TrackerData getAverage(const std::deque<TrackerData>& queue) {
//  return getAverageT(queue.begin(), queue.end());
/*
auto tsum =
    std::accumulate(queue.begin(), queue.end(), cv::Vec3d(0.0, 0.0, 0.0),
                    [](auto acc, auto i) { return acc + i.tvec; });
auto rsum =
    std::accumulate(queue.begin(), queue.end(), cv::Vec3d(0.0, 0.0, 0.0),
                    [](auto acc, auto i) { return acc + i.rvec; });
return TrackerData{tsum / static_cast<double>(queue.size()),
                   rsum / static_cast<double>(queue.size())};
*/
//}
/*
template <class Itr>
cv::Point2f getAverage(const Itr& first, const Itr& last) {
  const auto size = static_cast<double>(std::distance(first, last));
  return std::accumulate(first, last, cv::Point2f()) / size;
}
*/
void createAdjustedGray(cv::Mat& src, cv::Mat& dest) {
  dest = src.clone();
  return;
  cv::Mat gray;
  cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
  cv::equalizeHist(gray, dest);
}

// a,bÇåãÇ‘ê¸ï™Ç…pÇ©ÇÁç~ÇÎÇµÇΩêÇê¸ÇÃí∑Ç≥ÇãÅÇﬂÇÈ
double getDistance(const cv::Vec3d& a, const cv::Vec3d& b, const cv::Vec3d& p) {
  auto b2a = a - b;
  auto n = cv::normalize(b2a);
  auto p2a = a - p;
  auto v = p2a - p2a.dot(n) * n;
  return cv::norm(v);
}

int getIndexByID(int id, const std::vector<int>& ids) {
  const auto itr = std::find(ids.begin(), ids.end(), id);
  return std::distance(ids.begin(), itr);
}

TrackerData getTrackerDataByID(int id, const std::vector<int>& ids,
                               const std::vector<cv::Vec3d>& rvecs,
                               const std::vector<cv::Vec3d>& tvecs) {
  const auto index = getIndexByID(id, ids);
  return TrackerData{tvecs[index], rvecs[index]};
}

std::vector<cv::Point2f> getCornersByID(
    int id, const std::vector<int>& ids,
    const std::vector<std::vector<cv::Point2f>>& corners) {
  const auto index = getIndexByID(id, ids);
  return corners[index];
}

std::vector<cv::Point2f> getImagePoints(const cv::Vec3d& hips,
                                        const cv::Vec3d& lleg,
                                        const cv::Vec3d& rleg,

                                        const cv::Mat& cameraMatrix,
                                        const cv::Mat& distCoeffs) {
  std::vector<cv::Point3f> objectPoints;
  objectPoints.push_back(cv::Point3f(hips));
  objectPoints.push_back(cv::Point3f(lleg));
  objectPoints.push_back(cv::Point3f(rleg));
  std::vector<cv::Point2f> result;
  cv::projectPoints(objectPoints, cv::Vec3d(), cv::Vec3d(), cameraMatrix,
                    distCoeffs, result);
  return result;
}

cv::Affine3d selectCameraAffine(const std::vector<cv::Vec3d>& rvecs,
                                const std::vector<cv::Vec3d>& tvecs,
                                const TrackerData& hips,
                                const TrackerData& lleg,
                                const TrackerData& rleg) {
  cv::Affine3d cameraAffine;
  for (auto i = 0; i < rvecs.size(); i++) {
    cameraAffine = cv::Affine3d(rvecs[i], tvecs[i]).inv();
    cameraAffine =
        cv::Affine3d(rvecs[i]).inv() * cv::Affine3d(cv::Vec3d(), -tvecs[i]);
    auto h = cameraAffine * hips.tvec;
    auto l = cameraAffine * lleg.tvec;
    auto r = cameraAffine * rleg.tvec;
    if (!(r[0] - l[0] > 0.0)) {
      continue;
    }
    if (!((h[1] - l[1] > 0.0) && (h[1] - r[1] > 0.0))) {
      continue;
    }
    return cameraAffine;
  }
  return cameraAffine;
}

cv::Affine3d estimateCameraAffine3d(const Eigen::Vector3d& hmdPos,
                                    const Eigen::Quaterniond& hmdQuat,
                                    const TrackerData& hips,
                                    const TrackerData& lleg,
                                    const TrackerData& rleg) {
  // Todo hmdÇÃyé≤âÒì](yaw)ÇÃîΩâf
  const auto affineToOrigin =
      getAffine3dToOrigin(hips.tvec, lleg.tvec, rleg.tvec);
  auto t = affineToOrigin.translation();
  auto r = affineToOrigin.rvec();
  auto h = affineToOrigin * hips.tvec;
  auto hipso = (affineToOrigin * hips.affine3d()).translation();
  auto llego = (affineToOrigin * lleg.affine3d()).translation();
  auto rlego = (affineToOrigin * rleg.affine3d()).translation();

  const auto newTranslation =
      affineToOrigin.translation() + cv::Vec3d(hmdPos.x(), 0.0, hmdPos.z());
  return cv::Affine3d(affineToOrigin.rotation(), newTranslation);
}

cv::Mat squareResize(const cv::Mat& img) {
  const auto length = std::max(img.rows, img.cols);
  auto tx = 0, ty = 0;
  if (img.rows > img.cols) {
    tx = abs(img.cols - img.rows) / 2;
  } else {
    ty = abs(img.cols - img.rows) / 2;
  }
  cv::Mat dest = cv::Mat::zeros(length, length, CV_8UC3);
  cv::Mat mat = (cv::Mat_<double>(2, 3) << 1.0, 0.0, tx, 0.0, 1.0, ty);
  cv::warpAffine(img, dest, mat, dest.size());
  return dest;
}

Eigen::Quaterniond getMedian(
    std::deque<Eigen::Quaterniond> quatQueue) {  // ÉRÉsÅ[Ç≈ê≥ÇµÇ¢
  auto compare = [](const Eigen::Quaterniond& l, const Eigen::Quaterniond& r) {
    auto lAngle = l.angularDistance(Eigen::Quaterniond::Identity());
    auto rAngle = l.angularDistance(Eigen::Quaterniond::Identity());
    return lAngle < rAngle;
  };
  if (quatQueue.size() % 2 == 0) {
    auto i = quatQueue.size() / 2 - 1;
    auto j = i + 1;
    std::nth_element(quatQueue.begin(), quatQueue.begin() + i, quatQueue.end(),
                     compare);
    auto qi = *(quatQueue.begin() + i);
    std::nth_element(quatQueue.begin(), quatQueue.begin() + j, quatQueue.end(),
                     compare);
    auto qj = *(quatQueue.begin() + j);
    return qi.slerp(0.5, qj);
  } else {
    auto i = quatQueue.size() / 2;
    std::nth_element(quatQueue.begin(), quatQueue.begin() + i, quatQueue.end(),
                     compare);
    return *(quatQueue.begin() + i);
  }
}

void captureThreadFunction(concurrent_queue<cv::Mat>& imageQueue,
                           const TrackingConfiguration& config,
                           CameraCaptureBase* camera) {
  const auto height = camera->getHeight();
  const auto width = camera->getWidth();
  while (!shouldExit) {
    if (config.useSteamVr) {
      if (vr::VRCompositor() == NULL) {
        std::cout << "Waiting for VRCompositor..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        continue;
      }
    }
    cv::Mat image(height, width, CV_8UC3);
    camera->getImage(image);
    if (imageQueue.size() < 2) {
      imageQueue.push(image);
    }
  }
}

class ArUcoFrame {
 public:
  std::chrono::steady_clock::time_point timestamp;
  cv::Mat image;
  std::unique_ptr<TrackerData> hips;
  std::unique_ptr<TrackerData> lleg;
  std::unique_ptr<TrackerData> rleg;
};

void arThreadFunction(concurrent_queue<cv::Mat>& imageQueue,
                      concurrent_queue<std::unique_ptr<ArUcoFrame>>& arQueue,
                      const cv::Mat cameraMatrix, const cv::Mat distCoeffs,
                      const std::vector<cv::Ptr<cv::aruco::Board>>& boards) {
  cv::Ptr<cv::aruco::Dictionary> dictionary = boards[0]->dictionary;
  auto detectorParams = cv::aruco::DetectorParameters::create();
  detectorParams->detectInvertedMarker = true;
  // detectorParams->cornerRefinementMethod =
  // cv::aruco::CORNER_REFINE_APRILTAG;
  detectorParams->aprilTagMinClusterPixels = 0;
  detectorParams->aprilTagCriticalRad = 0.0;
  detectorParams->aprilTagDeglitch = 0.8;
  detectorParams->aprilTagQuadSigma = 0.8;
  detectorParams->aprilTagMaxLineFitMse = 1;
  ThreadPool threadPool(std::thread::hardware_concurrency());
  while (!shouldExit) {
    cv::Mat f;
    imageQueue.wait_and_pop(f);

    // throttle thread pool input
    if (threadPool.remaining_tasks_size() > threadPool.workers_size() * 2) {
      continue;
    }
    threadPool.enqueue(
        [&](cv::Mat image) {
          std::vector<int> ids;
          std::vector<std::vector<cv::Point2f>> corners, rejectedCandidates;
          cv::aruco::detectMarkers(image, dictionary, corners, ids,
                                   detectorParams, rejectedCandidates,
                                   cameraMatrix, distCoeffs);
          refineDetectedMarkers(image, boards.begin(), boards.end(), corners,
                                ids, rejectedCandidates, cameraMatrix,
                                distCoeffs);
          auto frame = std::make_unique<ArUcoFrame>();
          frame->image = image;
          frame->timestamp = std::chrono::steady_clock::now();
          if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(image, corners, ids);
            std::vector<cv::Vec3d> rvecs, tvecs;
            std::vector<int> results;
            estimatePoseBoards(corners, ids, boards.begin(), boards.end(),
                               cameraMatrix, distCoeffs, rvecs, tvecs, results);
            if (results[0]) {
              frame->lleg = std::make_unique<TrackerData>(rvecs[0], tvecs[0]);
            }
            if (results[1]) {
              frame->rleg = std::make_unique<TrackerData>(rvecs[1], tvecs[1]);
            }
            if (results[2]) {
              frame->hips = std::make_unique<TrackerData>(rvecs[2], tvecs[2]);
            }
          }
          arQueue.pushWithMove(std::move(frame));
        },
        f);
  }
}

class PublishData {
 public:
  std::pair<cv::Vec3d, Eigen::Quaterniond> hips;
  std::pair<cv::Vec3d, Eigen::Quaterniond> lleg;
  std::pair<cv::Vec3d, Eigen::Quaterniond> rleg;
};

void trackingThreadFunction(
    const TrackingConfiguration& config,
    concurrent_queue<std::unique_ptr<ArUcoFrame>>& imageQueue,
    concurrent_queue<std::unique_ptr<PublishData>>& dataQueue,
    CameraCaptureBase* camera, const cv::Mat cameraMatrix,
    const cv::Mat distCoeffs,
    const std::vector<cv::Ptr<cv::aruco::Board>>& boards) {
  auto firstFrame = imageQueue.wait_and_pop();
  constexpr auto windowName = "camera";
  const auto div = firstFrame->image.cols >= 1920 ? 2 : 1;

  auto windowWidth = firstFrame->image.cols / div;
  auto windowHeight = firstFrame->image.rows / div;
  cv::namedWindow(windowName);
  cv::resizeWindow(windowName, cv::Size(windowWidth, windowHeight));
  int exposure = 4;
  const auto exposureTrackbarHandler = [](int pos, void* data) {
    auto camera = reinterpret_cast<CameraCaptureBase*>(data);
    camera->setExposure(static_cast<double>(pos * 10));
  };
  exposureTrackbarHandler(exposure, camera);
  cv::createTrackbar("Exposure", windowName, &exposure, 10,
                     exposureTrackbarHandler, camera);

  cv::Affine3d cameraAffine(cv::Vec3d(0.0, 0.0, 0.0));
  std::vector<TrackerData> trackers(3);

  std::vector<cv::KalmanFilter> posKFs(3);
  for (auto& kf : posKFs) {
    constexpr auto type = CV_32F;
    kf.init(6, 3, 0, type);
    kf.measurementMatrix = cv::Mat::eye(3, 6, type);
  }
  int kfMode = 1;
  const auto kfTrackbarHandler = [](int pos, void* data) {
    auto kfs = reinterpret_cast<decltype(posKFs)*>(data);
    for (auto& kf : *kfs) {
      switch (pos) {
        case 0:
          cv::setIdentity(kf.processNoiseCov, cv::Scalar::all(1e-1));
          cv::setIdentity(kf.measurementNoiseCov, cv::Scalar::all(1e-1));
          cv::setIdentity(kf.errorCovPost, cv::Scalar::all(1e-1));
          break;
        case 1:
          cv::setIdentity(kf.processNoiseCov, cv::Scalar::all(1e-4));
          cv::setIdentity(kf.measurementNoiseCov, cv::Scalar::all(1e-3));
          cv::setIdentity(kf.errorCovPost, cv::Scalar::all(1e-3));
          break;
        default:
          break;
      }
    }
  };
  kfTrackbarHandler(kfMode, &posKFs);
  cv::createTrackbar("KF params", windowName, &kfMode, 1, kfTrackbarHandler,
                     &posKFs);
  std::vector<LowPassFilter<cv::Vec3d>> posLPFs(3);
  auto posGain = 30;
  const auto posTrackbarHandler = [](auto pos, auto data) {
    auto filters = reinterpret_cast<decltype(posLPFs)*>(data);
    auto gain = (double)pos / 100.0;
    for (auto& f : *filters) {
      f.gain = gain;
    }
  };
  posTrackbarHandler(posGain, &posLPFs);
  cv::createTrackbar("Pos LPF", windowName, &posGain, 99, posTrackbarHandler,
                     &posLPFs);

  std::vector<LowPassFilter<Eigen::Quaterniond>> quatLPFs(boards.size());
  auto quatGain = 70;
  const auto quatTrackbarHandler = [](auto pos, auto data) {
    auto filters = reinterpret_cast<decltype(quatLPFs)*>(data);
    auto gain = (double)pos / 100.0;
    for (auto& f : *filters) {
      f.gain = gain;
    }
  };
  quatTrackbarHandler(quatGain, &quatLPFs);
  cv::createTrackbar("Quat LPF", windowName, &quatGain, 99, quatTrackbarHandler,
                     &quatLPFs);

  auto maxPastData = 3;
  std::vector<std::deque<cv::Vec3d>> pastPositions(boards.size());
  for (auto& queue : pastPositions) {
    queue.push_back(cv::Vec3d());
  }
  std::vector<std::deque<Eigen::Quaterniond>> pastQuats(boards.size());
  for (auto& queue : pastQuats) {
    queue.push_back(Eigen::Quaterniond::Identity());
  }
  cv::createTrackbar("Past Data", windowName, &maxPastData, 9);

  auto previewRotateCode = 3;
  cv::createTrackbar("Rotation", windowName, &previewRotateCode, 3);

  auto frameStart = std::chrono::steady_clock::now();
  auto isFirstFrame = true;
  auto lastFrameTimestamp = firstFrame->timestamp;
  std::deque<double> frameTimes;
  frameTimes.push_back(0.0);
  while (!shouldExit) {
    auto startTime =
        getDurationInMillis(frameStart, std::chrono::steady_clock::now());
    PRINT(startTime);
    auto frame = imageQueue.wait_and_pop();
    if (frame->timestamp < lastFrameTimestamp) {
      continue;
    }
    lastFrameTimestamp = frame->timestamp;
    auto captureTime =
        getDurationInMillis(frameStart, std::chrono::steady_clock::now());
    PRINT(captureTime);

    cv::Mat renderer(frame->image);

    std::vector<int> results(boards.size());

    auto canCalibrate = (frame->hips != nullptr && frame->lleg != nullptr &&
                         frame->rleg != nullptr);
    if (frame->lleg != nullptr) {
      trackers[0] = *frame->lleg;
      results[0] = true;
    } else {
      results[0] = false;
    }
    if (frame->rleg != nullptr) {
      trackers[1] = *frame->rleg;
      results[1] = true;
    } else {
      results[1] = false;
    }
    if (frame->hips != nullptr) {
      trackers[2] = *frame->hips;
      results[2] = true;
    } else {
      results[2] = false;
    }

    {
      if (isFirstFrame && canCalibrate) {
        for (auto i = 0; i < boards.size(); i++) {
          posKFs[i].statePre.at<float>(0) = trackers[i].tvec[0];
          posKFs[i].statePre.at<float>(1) = trackers[i].tvec[1];
          posKFs[i].statePre.at<float>(2) = trackers[i].tvec[2];
          posKFs[i].statePre.at<float>(3) = 0.0;
          posKFs[i].statePre.at<float>(4) = 0.0;
          posKFs[i].statePre.at<float>(5) = 0.0;
          quatLPFs[i].lastValue = rvecToQuaternion(trackers[i].rvec);
        }
        isFirstFrame = false;
      }

      std::vector<cv::Vec3d> pos(boards.size());
      std::vector<Eigen::Quaterniond> quat(boards.size());
      for (auto i = 0; i < boards.size(); i++) {
        cv::Mat kfResult;
        kfResult = posKFs[i].predict();
        auto isFound = results[i] > 0;
        if (isFound) {
          pastPositions[i].push_back(trackers[i].tvec);
          while (pastPositions[i].size() > std::max(maxPastData, 1)) {
            pastPositions[i].pop_front();
          }
          auto med = getMedian(pastPositions[i]);
          cv::Mat measure = (cv::Mat_<float>(3, 1) << med[0], med[1], med[2]);
          kfResult = posKFs[i].correct(measure);
        }
        cv::Vec3d p = {kfResult.at<float>(0), kfResult.at<float>(1),
                       kfResult.at<float>(2)};
        pos[i] = posLPFs[i].process(p);

        Eigen::Quaterniond q;
        if (isFound) {
          q = rvecToQuaternion(trackers[i].rvec);
          pastQuats[i].push_back(q);
          while (pastQuats[i].size() > std::max(maxPastData, 1)) {
            pastQuats[i].pop_front();
          }
        }
        quat[i] = quatLPFs[i].process(getMedian(pastQuats[i]));
        cv::aruco::drawAxis(renderer, cameraMatrix, distCoeffs,
                            quaternionToRvec(quat[i]), pos[i], 0.1f);
      }
      if (config.useSteamVr) {
        auto publishStart =
            getDurationInMillis(frameStart, std::chrono::steady_clock::now());
        PRINT(publishStart);
        auto hmd = getHmdPose();
        if (hmd) {
          auto& poseMat = hmd->mDeviceToAbsoluteTracking;
          hmdPos = getTranslation(poseMat);
          hmdQuat = Eigen::Quaterniond(getRotationMatrix(poseMat));
        }
        auto finalHips =
            getTrackerData(cameraAffine, pos[2], quaternionToRvec(quat[2]));
        auto finalLleg =
            getTrackerData(cameraAffine, pos[0], quaternionToRvec(quat[0]));
        auto finalRleg =
            getTrackerData(cameraAffine, pos[1], quaternionToRvec(quat[1]));
        auto data = std::make_unique<PublishData>();
        data->hips = finalHips;
        data->lleg = finalLleg;
        data->rleg = finalRleg;
        dataQueue.pushWithMove(std::move(data));
        auto publishEnd =
            getDurationInMillis(frameStart, std::chrono::steady_clock::now());
        PRINT(publishEnd);
      }
    }

    auto averageFrameTime =
        std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0) /
        (double)(frameTimes.size());
    auto averageFPS = 1000.0 / averageFrameTime;
    std::ostringstream fpsText("");
    fpsText << "Average: " << averageFPS << " fps "
            << "(" << averageFrameTime << "ms)";
    cv::Mat scale, dest;
    cv::resize(renderer, scale, cv::Size(windowWidth, windowHeight));
    if (previewRotateCode < 3) {
      cv::rotate(scale, dest, previewRotateCode);
    } else {
      dest = scale.clone();
    }
    if (dest.cols < dest.rows) {
      dest = squareResize(dest);
    }
    cv::putText(dest, fpsText.str(), cv::Point(10, 20), cv::FONT_HERSHEY_DUPLEX,
                0.5, cv::Scalar(0, 0, 255));

    cv::imshow(windowName, dest);
    auto key = cv::waitKey(1);
    if (key == 27) {  // esc
      break;
    }
    if (key == 'c' && canCalibrate) {
      std::lock_guard<std::mutex> lock(tracker_mtx);
      cameraAffine = estimateCameraAffine3d(hmdPos, hmdQuat, trackers[2],
                                            trackers[0], trackers[1]);
    }
    auto uiTime =
        getDurationInMillis(frameStart, std::chrono::steady_clock::now());
    PRINT(uiTime);

    auto endTime =
        getDurationInMillis(frameStart, std::chrono::steady_clock::now());
    PRINT(endTime);
    frameTimes.push_back(endTime);
    if (frameTimes.size() > 100) {
      frameTimes.pop_front();
    }
    frameStart = std::chrono::steady_clock::now();
  }
}

void publishThreadFunction(
    const TrackingConfiguration& config,
    concurrent_queue<std::unique_ptr<PublishData>>& dataQueue) {
  if (!config.useSteamVr) {
    return;
  }
  while (!shouldExit) {
    if (dataQueue.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    auto data = dataQueue.wait_and_pop();
    setVirtualDevicePosition(transmitSocket, leftFootID, data->lleg.first,
                             data->lleg.second);
    setVirtualDevicePosition(transmitSocket, rightFootID, data->rleg.first,
                             data->rleg.second);
    setVirtualDevicePosition(transmitSocket, hipID, data->hips.first,
                             data->hips.second);
  }
}

std::string decodeFourcc(double fourcc) {
  int ex = static_cast<int>(fourcc);
  char EXT[] = {(char)(ex & 0XFF), (char)((ex & 0XFF00) >> 8),
                (char)((ex & 0XFF0000) >> 16), (char)((ex & 0XFF000000) >> 24),
                0};
  return EXT;
}

int main(int argc, char** argv) {
  TrackingConfiguration config;
  constexpr auto keys =
      "{help h usage ? |             | Print this message }"
      "{ci camera-id   |0            | Camera ID to capture }"
      "{camera         |camera.yaml  | Camera parameter file }"
      "{fps            |0.0          | Capture frame rate }"
      "{marker         |markers.yaml | Marker parameter file }"
      "{ps-eye         |false        | Use PlayStation Eye }"
      "{no-steam-vr    |false        | Don't use SteamVR }";
  cv::CommandLineParser parser(argc, argv, keys);
  if (parser.has("help")) {
    parser.printMessage();
    return 0;
  }
  const auto cameraID = parser.get<int>("ci");
  const auto cameraFile = parser.get<std::string>("camera");
  const auto markerFile = parser.get<std::string>("marker");
  config.useSteamVr = !parser.get<bool>("no-steam-vr");
  config.usePSEye = parser.get<bool>("ps-eye");
  if (!parser.check()) {
    parser.printErrors();
    return 0;
  }
  std::unique_ptr<CameraCaptureBase> camera;
  if (config.usePSEye) {
    camera = decltype(camera)(new HighguiCapture(cameraID));
    config.fps = 60.0;
  } else {
    camera = decltype(camera)(new CameraCapture(cameraID));
    config.fps = parser.get<double>("fps");
  }
  cv::FileStorage cameraFS(cameraFile, cv::FileStorage::READ);
  if (!cameraFS.isOpened()) {
    std::cout << "Failed to open " << cameraFile << std::endl;
    return 1;
  }
  const auto cameraMatrix = cameraFS["camera_matrix"].mat();
  const auto distCoeffs = cameraFS["distortion_coefficients"].mat();
  const int imageWidth = cameraFS["image_width"];
  const int imageHeight = cameraFS["image_height"];
  cameraFS.release();

  const auto r = camera->Open(imageWidth, imageHeight, config.fps);
  if (r != 0) {
    PRINT(r);
    std::cout << "Failed to open camera" << std::endl;
    return 1;
  }

  const cv::FileStorage markerFS(markerFile, cv::FileStorage::READ);
  const auto dictID = (int)markerFS["dictionary"];
  const auto dictionary = cv::aruco::getPredefinedDictionary(dictID);
  std::vector<cv::Ptr<cv::aruco::Board>> boards;
  markerFS["boards"] >> boards;
  for (auto& b : boards) {
    b->dictionary = dictionary;
  }

  if (config.useSteamVr) {
    // Initialize stuff
    vr::EVRInitError error = vr::VRInitError_Compositor_Failed;
    std::cout << "Looking for SteamVR..." << std::flush;
    while (error != vr::VRInitError_None) {
      m_VRSystem = vr::VR_Init(&error, vr::VRApplication_Overlay);
      if (error != vr::VRInitError_None) {
        std::cout << "\nFailed due to reason "
                  << VR_GetVRInitErrorAsSymbol(error) << "\n"
                  << std::flush;
        std::cout << "Trying again in a few seconds...\n" << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(4));
      }
    }
    std::cout << "Success!\n";
    findTrackers();
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  concurrent_queue<cv::Mat> imageQueue;
  concurrent_queue<std::unique_ptr<ArUcoFrame>> arQueue;
  concurrent_queue<std::unique_ptr<PublishData>> dataQueue;
  std::thread capture_thread(captureThreadFunction, std::ref(imageQueue),
                             std::ref(config), camera.get());
  std::thread ar_thread(arThreadFunction, std::ref(imageQueue),
                        std::ref(arQueue), cameraMatrix, distCoeffs,
                        std::ref(boards));
  std::thread publish_thread(publishThreadFunction, std::ref(config),
                             std::ref(dataQueue));
  // std::thread zmq_thread(zmqThreadFunction, std::ref(subscriber));
  trackingThreadFunction(std::ref(config), std::ref(arQueue),
                         std::ref(dataQueue), camera.get(), cameraMatrix,
                         distCoeffs, boards);
  {
    std::lock_guard<std::mutex> lock(tracker_mtx);
    shouldExit = true;
  }
  // zmq_thread.join();
  capture_thread.join();
  ar_thread.join();
  publish_thread.join();
  return 0;
}
