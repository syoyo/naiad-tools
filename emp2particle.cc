//
// Copyright Syoyo Fujita, Light Transport Entertainment Inc.
// 

//
// Load emp particle and convert it into custom particle data.
// Particle data is compressed by lz4 to save storage.
//
// @todo { 
//  * Out-of-core particle processing. 
//  * Support custom particle attributes.
//  * Support multi-frame emp.
// }
// 

// To handle 2GB+ file.
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64


#include <NbEmpReader.h>
#include <NbFilename.h>
#include <NbFactory.h>
#include <NbBodyReader.h>
#include <NbSequenceReader.h>
#include <NbBody.h>
#include <NbString.h>
#include <em_log.h>             // NB_XYZ macros

#include <vector>
#include <map>

//#define ENABLE_LZ4_COMPRESS (1)

extern "C" {
#include "lz4.h"
}

static int
EstimateCompressedBufferSize(
  int inputSize)
{
  // From LZ4:
  // To avoid any problem, size it to handle worst cases situations (input data not compressible)
  // Worst case size is : "inputsize + 0.4%", with "0.4%" being at least 8 bytes.

  int k = (int)(inputSize * 0.05);  // for safety, +0.1% ;-)
  if (k < 8) k = 8;
  return inputSize + k;
}

class Particle
{
 public:
  Particle() {}
  ~Particle() {}

  bool Write(const char* filename) {
    FILE* fp = fopen(filename, "wb");
    assert(fp);
    unsigned int n = positions_.size();
    size_t sz = 0;
    sz = fwrite(&n, sizeof(unsigned int), 1, fp);
    assert(sz == 1);

#ifdef ENABLE_LZ4_COMPRESS
    {
      int bufsize = EstimateCompressedBufferSize(n * sizeof(float));
      std::vector<char> buffer(bufsize);
      int len = LZ4_compress(reinterpret_cast<char*>(&positions_[0]), &buffer[0], sizeof(float) * n);
      printf("LZ4 compress: %d bytes -> %d bytes\n", sizeof(float) * n, len);

      sz = fwrite(&len, sizeof(int), 1, fp);
      assert(sz == 1);

      sz = fwrite(&buffer[0], sizeof(char), len, fp);
      assert(sz == len);
    }
#else
    {
      sz = fwrite(&positions_[0], sizeof(float), n, fp);
      assert(sz == n);
    }
#endif

    fclose(fp);

    int Mparticles = (int)((double)positions_.size() / 3 / (1000.0 * 1000.0));
    if (Mparticles < 1) {
      std::cout << "Wrote " << positions_.size() / 3 << " particles data to " << filename << "\n";
    } else {
      std::cout << "Wrote " << Mparticles << " Mparticles data to " << filename << "\n";
    }

    return true;
  }

  std::vector<float> positions_;
  //std::vector<float> radiuses_;   // @todo
  //std::vector<float> colors_;     // @todo
  //std::vector<int>   ids_;        // @todo
};

static std::string
GetStringOfType(
  Nb::ValueBase::Type valueType)
{
  switch (valueType) {
  case Nb::ValueBase::FloatType:
    return "float";
  case Nb::ValueBase::IntType:
    return "int32";
  case Nb::ValueBase::Int64Type:
    return "int64";
  case Nb::ValueBase::Vec3fType:
    return "float3";
  case Nb::ValueBase::Vec3iType:
    return "int3";
  default:
    return "unsupported";
  }
}

static bool
Emp2Particle(
  Particle& particle,     // out
  const Nb::Body* body)   // in
{
  NB_INFO("EMP Process particle body(" << body->name() << ")...");
  const Nb::ParticleShape& particleShape(body->constParticleShape());
  const Nb::TileLayout& layout(body->constLayout());

  NB_INFO("  Block count: " << layout.fineTileCount());
  NB_INFO("  Channel count: " << particleShape.channelCount());

  //
  // List up channels
  //
  for (int channel = 0; channel < particleShape.channelCount(); channel++) {
    const Nb::ParticleChannelBase& empChannel(particleShape.constChannelBase(channel));

    NB_INFO("  Channel(" << channel << ") name = " << empChannel.name() << ", type = " << GetStringOfType(empChannel.type()));

  }

  //
  // Process position
  // 
  {
    const unsigned int blockCount = layout.fineTileCount();
    size_t particleCount = 0; // 64bit int in 64bit env.
    const em::block3_array3f& positionBlocks(particleShape.constBlocks3f("position"));

    for (unsigned int blockIndex = 0; blockIndex < blockCount; blockIndex++) {
      const em::block3vec3f& positionBlock(positionBlocks(blockIndex));
      size_t blockParticleCount = positionBlock.size();
      for (unsigned int p = 0; p < blockParticleCount; p++) {
        const em::vec3f& v = positionBlock(p);
        particle.positions_.push_back(v[0]);
        particle.positions_.push_back(v[1]);
        particle.positions_.push_back(v[2]);
      }
      
      particleCount += blockParticleCount;
    }

    NB_INFO("  # of position blocks = " << blockCount);
    NB_INFO("  # of particles = " << particleCount);

    particle.positions_.reserve(particleCount * 3);

  }
  //
  // @todo { Handle other fields. }
  // 
  return true;
}


static bool
ProcEmp(
  const std::string& filename)
{
  try {
    std::cout << "Reading " << filename << std::endl;
    Nb::EmpReader empReader(filename, "*", "Body"); // May throw.
    NB_INFO("EMP time: " << empReader.time());
    NB_INFO("EMP revision: " << empReader.revision());
    NB_INFO("EMP body count: " << empReader.bodyCount());
    // Assume single frame emp.
    //int frame = 0;
    //int timestep = 0;
    //Nb::extractFrameAndTimestep(filename, frame, timestep);
    //NB_INFO("Frame: " << frame);
    //NB_INFO("Timestep: " << timestep);
    const Nb::String sequenceName = Nb::hashifyFilename(filename);
    NB_INFO("Sequence name: '" << sequenceName);

    for (int i = 0; i < empReader.bodyCount(); i++) {
      const Nb::Body* body(empReader.ejectBody(i));
      NB_INFO("EMP body(" << i << ") name = " << body->name());

      // Process particle body only.
      if (body->hasShape("Particle")) {
        Particle particle;
        Emp2Particle(particle, body);
        char buf[4096];
        sprintf(buf, "particle_%03d.dat", i);
        particle.Write(buf);
      } else {
        NB_WARNING("EMP body(" << body->name() << ") is not a particle shape. Skipping.");
      }
    }
  } 
  catch (std::exception &ex) {
    NB_ERROR("exception: " << ex.what());
    return false;   // Failure.
  }
  catch (...) {
    NB_ERROR("unknown exception");
    return false;   // Failure.
  }

  return true;
}

int
main(
  int argc,
  char **argv)
{
  std::string input = "input.emp";

  if (argc > 1) {
    input = std::string(argv[1]);
  }

  // Must call Nb::begin() before all Nb API call.
  Nb::begin();

  bool ret = ProcEmp(input);

  // Also must call Nb::end() when process exits.
  Nb::end();

  return (int)ret;
}
