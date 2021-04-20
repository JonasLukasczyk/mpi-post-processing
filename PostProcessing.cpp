#include "PostProcessing.h"

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <ttkCinemaWriter.h>
#include <ttkArrayEditor.h>

int PostProcessing::init(
  const std::string outputPath
){
  // create cinema writer
  auto writer = vtkSmartPointer<ttkCinemaWriter>::New();
  writer->SetDatabasePath(outputPath);
  writer->SetFormat(ttkCinemaWriter::FORMAT::VTK);

  // delete database if exists already
  writer->DeleteDatabase();

  // create a lockfile to enable parallel mpi writes
  writer->InitializeLockFile();

  // return success
  return 1;
}

int PostProcessing::processRankData(
  const float* rankData,
  const int nComponents,
  const int nTuples,
  const int iRank,
  const int dim,
  const int rankDimZ,
  const int time,
  const std::string outputPath
) {

  // prepare image data output
  auto image = vtkSmartPointer<vtkImageData>::New();
  image->SetOrigin(
    0,
    0,
    iRank*(rankDimZ-1)
  );
  image->SetExtent(
    0, dim-1,
    0, dim-1,
    0, rankDimZ-1
  );

  // create an array with given data
  auto array = vtkSmartPointer<vtkFloatArray>::New();
  array->SetName("Field");
  array->SetVoidArray(
    static_cast<void*>(const_cast<float*>(rankData)),
    nTuples*nComponents,
    1
  );
  image->GetPointData()->AddArray(array);

  // add field data information about the current data product
  auto arrayEditor = vtkSmartPointer<ttkArrayEditor>::New();
  arrayEditor->SetInputDataObject(image);
  arrayEditor->SetEditorMode(0); // parse from String
  arrayEditor->SetDataString(
      "Rank,"+std::to_string(iRank)+"\n"
    + "Time,"+std::to_string(time)+"\n"
  );

  // create cinema writer
  auto writer = vtkSmartPointer<ttkCinemaWriter>::New();
  writer->SetDatabasePath(outputPath);
  writer->SetCompressionLevel(9);
  writer->SetInputConnection(arrayEditor->GetOutputPort());
  writer->Update();

  // return success
  return 1;
}

int PostProcessing::processCombinedData(
  const float* combinedData,
  const int nComponents,
  const int nTuples,
  const int dim,
  const int time,
  const std::string outputPath
) {

  ttk::globalDebugLevel_ = 3;

  // prepare image data output
  auto image = vtkSmartPointer<vtkImageData>::New();
  image->SetOrigin( 0, 0, 0 );
  image->SetDimensions( dim,dim,dim );

  // create an array with given data
  auto array = vtkSmartPointer<vtkFloatArray>::New();
  array->SetName("Field");
  array->SetVoidArray(
    static_cast<void*>(const_cast<float*>(combinedData)),
    nTuples*nComponents,
    1
  );
  image->GetPointData()->AddArray(array);

  // add field data information about the current data product
  auto arrayEditor = vtkSmartPointer<ttkArrayEditor>::New();
  arrayEditor->SetInputDataObject(image);
  arrayEditor->SetEditorMode(0); // parse from String
  arrayEditor->SetDataString(
    "Time,"+std::to_string(time)
  );

  // create cinema writer
  auto writer = vtkSmartPointer<ttkCinemaWriter>::New();
  writer->SetDatabasePath(outputPath);
  writer->SetCompressionLevel(9);
  writer->SetInputConnection(arrayEditor->GetOutputPort());
  writer->Update();

  // return success
  return 1;
}