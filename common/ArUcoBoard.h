#pragma once

#include <opencv2/aruco.hpp>
#include <opencv2/core/affine.hpp>
#include <opencv2/highgui.hpp>

template <class BoardIterator>
void estimatePoseBoards(cv::InputArrayOfArrays corners, cv::InputArray ids,
                        const BoardIterator& boardFirst,
                        const BoardIterator& boardLast,
                        cv::InputArray cameraMatrix, cv::InputArray distCoeffs,
                        std::vector<cv::Vec3d>& rvecs,
                        std::vector<cv::Vec3d>& tvecs,
                        std::vector<int>& results) {
  auto num = std::distance(boardFirst, boardLast);
  rvecs.resize(num);
  tvecs.resize(num);
  results.resize(num);
  auto i = 0;
  for (auto itr = boardFirst; itr != boardLast; ++itr) {
    results[i] = cv::aruco::estimatePoseBoard(corners, ids, *itr, cameraMatrix,
                                              distCoeffs, rvecs[i], tvecs[i]);
    i++;
  }
}

template <class BoardIterator>
void refineDetectedMarkers(cv::InputArray image,
                           const BoardIterator& boardFirst,
                           const BoardIterator& boardLast,
                           cv::InputOutputArrayOfArrays detectedCorners,
                           cv::InputOutputArray detectedIds,
                           cv::InputOutputArrayOfArrays rejectedCorners,
                           cv::InputArray cameraMatrix = noArray(),
                           cv::InputArray distCoeffs = noArray()) {
  for (auto itr = boardFirst; itr != boardLast; ++itr) {
    cv::aruco::refineDetectedMarkers(image, *itr, detectedCorners, detectedIds,
                                     rejectedCorners, cameraMatrix, distCoeffs);
  }
}

cv::Ptr<cv::aruco::Board> createBoardCube(
    const std::vector<cv::Ptr<cv::aruco::Board>>& boards,
    const std::vector<cv::Affine3d>& poses);
/*
class SerializableArUcoBoard : public cv::aruco::Board {
 public:
  SerializableArUcoBoard() {}
  SerializableArUcoBoard(const cv::aruco::Board& board);
  void write(cv::FileStorage& fs) const;
  void read(const cv::FileNode& node);
};

void write(cv::FileStorage& fs, const std::string&, const
SerializableArUcoBoard& x); void read(const cv::FileNode& node,
SerializableArUcoBoard& x, const SerializableArUcoBoard& default_value =
SerializableArUcoBoard());
*/

namespace cv {
void write(cv::FileStorage& fs, const std::string&,
           const cv::aruco::Dictionary& dictionary);
void read(const FileNode& node, cv::aruco::Dictionary& dictionary,
          const cv::aruco::Dictionary& default_value = cv::aruco::Dictionary());
void write(cv::FileStorage& fs, const std::string&,
           const cv::aruco::Board& board);
void read(const FileNode& node, cv::aruco::Board& board,
          const cv::aruco::Board& default_value = cv::aruco::Board());
void write(cv::FileStorage& fs, const std::string&,
           const cv::Ptr<cv::aruco::Board>& board);
void read(const FileNode& node, cv::Ptr<cv::aruco::Board>& board,
          const cv::Ptr<cv::aruco::Board>& default_value =
              cv::Ptr<cv::aruco::Board>(new cv::aruco::Board()));
}  // namespace cv
