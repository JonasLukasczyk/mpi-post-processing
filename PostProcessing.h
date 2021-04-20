#pragma once

#include <string>

namespace PostProcessing {

  int init(const std::string outputPath);

  int processRankData(
    const float* rankData,
    const int nComponents,
    const int nTuples,
    const int iRank,
    const int dim,
    const int rankDimZ,
    const int time,
    const std::string outputPath
  );

  int processCombinedData(
    const float* combinedData,
    const int nComponents,
    const int nTuples,
    const int dim,
    const int time,
    const std::string outputPath
  );

};