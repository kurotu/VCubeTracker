#include "ArUcoBoard.h"

cv::Ptr<cv::aruco::Board> applyPoseToBoard(cv::Ptr<cv::aruco::Board> board,
                                           cv::Affine3d pose) {
  auto b =
      cv::aruco::Board::create(board->objPoints, board->dictionary, board->ids);
  for (auto i = 0; i < b->objPoints.size(); i++) {
    std::transform(board->objPoints[i].begin(), board->objPoints[i].end(),
                   b->objPoints[i].begin(),
                   [&pose](auto p) { return pose * p; });
  }
  return b;
}

cv::Ptr<cv::aruco::Board> createBoardCube(
    const std::vector<cv::Ptr<cv::aruco::Board>>& boards,
    const std::vector<cv::Affine3d>& poses) {
  auto appliedBoard = applyPoseToBoard(boards[0], poses[0]);
  auto cube = cv::aruco::Board::create(appliedBoard->objPoints,
                                       boards[0]->dictionary, boards[0]->ids);
  for (auto i = 1; i < boards.size(); i++) {
    auto& board = boards[i];
    cube->ids.insert(cube->ids.end(), board->ids.begin(), board->ids.end());
    auto objPoints = applyPoseToBoard(board, poses[i])->objPoints;
    cube->objPoints.insert(cube->objPoints.end(), objPoints.begin(),
                           objPoints.end());
  }
  return cube;
}

void cv::write(cv::FileStorage& fs, const std::string&,
               const cv::aruco::Dictionary& dictionary) {
  fs << "{"
     // clang-format off
     << "bytesList" << dictionary.bytesList
     << "markerSize" << dictionary.markerSize
     << "maxCorrectionBits" << dictionary.maxCorrectionBits
     // clang-format on
     << "}";
}

void cv::read(const FileNode& node, cv::aruco::Dictionary& dictionary,
              const cv::aruco::Dictionary& default_value) {
  if (node.empty()) {
    dictionary = default_value;
  } else {
    node["bytesList"] >> dictionary.bytesList;
    node["markerSize"] >> dictionary.markerSize;
    node["maxCorrectionBits"] >> dictionary.maxCorrectionBits;
  }
}

void cv::write(cv::FileStorage& fs, const std::string&,
               const cv::aruco::Board& board) {
  fs << "{"
     // clang-format off
     << "ids" << board.ids
     << "dictionary" << *(board.dictionary) 
     << "objPoints" << board.objPoints
     // clang-format on
     << "}";
}

void cv::read(const FileNode& node, cv::aruco::Board& board,
              const cv::aruco::Board& default_value) {
  if (node.empty()) {
    board = default_value;
  } else {
    node["ids"] >> board.ids;
    board.dictionary =
        cv::Ptr<cv::aruco::Dictionary>(new cv::aruco::Dictionary());
    node["dictionary"] >> *(board.dictionary);
    node["objPoints"] >> board.objPoints;
  }
}

void cv::write(cv::FileStorage& fs, const std::string& str,
               const cv::Ptr<cv::aruco::Board>& board) {
  write(fs, str, *board);
}

void cv::read(const FileNode& node, cv::Ptr<cv::aruco::Board>& board,
              const cv::Ptr<cv::aruco::Board>& default_value) {
  if (node.empty()) {
    board = default_value;
  } else {
    board = cv::Ptr<cv::aruco::Board>(new cv::aruco::Board());
    read(node, *board);
  }
}
