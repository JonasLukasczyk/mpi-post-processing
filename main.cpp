#include <mpi.h>

#include <stdio.h>
#include <fstream>
#include <cmath>

#include <PerlinNoise.h>
#include <PostProcessing.h>


int compute(
  std::vector<float>& data,
  const int iRank,
  const int dim,
  const int rankDimZ,
  const int time
){
  const int offset = rankDimZ*iRank;
  // const int z0 = rankDimZ*iRank + (iRank>0 ? -iRank : 0);
  const int z0 = rankDimZ*iRank;
  const int zN = z0+rankDimZ;

  PerlinNoise pn(0);

  // const double dimD = (((double)dim)-1.0)/10.0;
  const double dimD = (((double)dim)-1.0)/10.0;

  for(int x=0; x<dim; x++){
    for(int y=0; y<dim; y++){
      for(int z=z0; z<zN; z++){
        const int idx = (z-z0)*dim*dim+y*dim + x;
        double u = ((double)x)/dimD;
        double v = ((double)y)/dimD;
        double w = ((double)z)/dimD;
        double t = ((double)time)/dimD;
        data[idx] = pn.noise(u,v,w+t);
      }
    }
  }

  return 1;
}

int writeToDisk(const std::string& path, const float* data, const int nValues){
  std::ofstream out;
  out.open( path, std::ios::out | std::ios::binary);

  if(!out.is_open())
    return 0;

  out.write( reinterpret_cast<const char*>( data ), sizeof( float )*nValues);

  out.close();

  return 1;
}

int main(int argc, char** argv) {
  MPI_Init(NULL, NULL);

  int nRanks;
  MPI_Comm_size(MPI_COMM_WORLD, &nRanks);

  int iRank;
  MPI_Comm_rank(MPI_COMM_WORLD, &iRank);

  if(argc!=4){
    if(iRank==0){
      std::cout<<"This simulation requires the following parameters in that order:\n";
      std::cout<<" (int) Dimension in xyz \n";
      std::cout<<" (int) Number of Timesteps\n";
      std::cout<<" (String) Output Cinema Database Path (needs to end with .cdb)\n";
      std::cout<<std::endl;
    }
    return 0;
  }

  // same dim in xyz
  const int dim = std::stoi(argv[1]);

  // z-dim is split based on rank
  const int rankDimZ = dim/nRanks;

  // number of timesteps
  const int nTimesteps = std::stoi(argv[2]);

  // output path for cinema database
  const std::string outputPath = std::string(argv[3]);

  int status = 0;

  // if first rank then print parameters and initialize post processing (prepare database etc)
  if(iRank==0){
    std::cout<<"Dim:       "<<dim<<"^3"<<std::endl;
    std::cout<<"Timesteps: "<<nTimesteps<<std::endl;
    std::cout<<"Ranks:     "<<nRanks<<std::endl;
    std::cout<<"RankDimZ:  "<<rankDimZ<<std::endl;
    std::cout<<"Output:    "<<outputPath<<std::endl;

    status = PostProcessing::init(
      outputPath
    );
    if(!status){
      std::cout<<"ERROR in initialization of Post Processing"<<std::endl;
      return 1;
    }
  }

  const int nRankValues = dim*dim*rankDimZ;
  const int nCombinedValues = nRankValues*nRanks;
  std::vector<float> rankData(nRankValues);

  // on first rank initialize combined data buffer
  std::vector<float> combinedData;
  if(iRank==0)
    combinedData.resize(nCombinedValues);

  MPI_Barrier(MPI_COMM_WORLD);

  // TODO: Pseudo Code
  // when simulation of one timestep is complete:
  //   * on each rank resample simulation data on 2D uniform grid (called tile)
  //   * gather all tiles on one rank (bad: but ok for now)
  //   * on this one rank combine all ranks into one huge 2D grid
  //   * send combined grid to post processing module

  for(int t=0; t<nTimesteps; t++){
    // proxy simulation
    status = compute(
      rankData,
      iRank,
      dim,
      rankDimZ,
      t
    );
    if(!status){
      std::cout<<"ERROR in Timestep Computation"<<std::endl;
      return 1;
    }

    if(iRank==0){
      std::cout<<"Timestep "<<(t+1)<<"/"<<nTimesteps<<" complete"<<std::endl;
    }

    // gather all data on rank 0
    MPI_Gather(
      rankData.data(),
      nRankValues,
      MPI_FLOAT,
      iRank==0 ? combinedData.data() : NULL,
      nRankValues,
      MPI_FLOAT,
      0,
      MPI_COMM_WORLD
    );

    // rank 0 performs post processing
    if(iRank==0){
      status = PostProcessing::processCombinedData(
        combinedData.data(),
        1,
        nCombinedValues,
        dim,
        t,
        outputPath
      );
      if(!status){
        std::cout<<"ERROR in Post Processing"<<std::endl;
        return 1;
      }
    }

    // status = writeToDisk(
    //   outputPath+"_"+std::to_string(iRank)+"_"+std::to_string(t)+".bin",
    //   data.data(),
    //   nValues
    // );
    // if(!status){
    //   std::cerr<<"ERROR"<<std::endl;
    //   return 1;
    // }
  }

  MPI_Finalize();
}