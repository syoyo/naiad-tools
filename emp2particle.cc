//
// Copyright Light Transport Entertainment Inc.
// 

#include <NbEmpReader.h>
#include <NbFilename.h>
#include <NbFactory.h>
#include <NbBodyReader.h>
#include <NbSequenceReader.h>
#include <NbBody.h>
#include <NbString.h>
#include <em_log.h>             // NB_XYZ macros

bool
Emp2Particle(
)
{
  // @todo
  return true;
}


bool
ProcEmp(
  const std::string& filename)
{
  try {
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
    NB_INFO("Sequence name: '" << sequenceName << "'\n");

    for (int i = 0; i < empReader.bodyCount(); i++) {
      const Nb::Body* body(empReader.ejectBody(i));
      NB_INFO("EMP body(" << i << ") name = " << body->name());
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

  // Must call Nb::begin() before all Nb API call.
  Nb::begin();

  bool ret = ProcEmp(input);

  // Also must call Nb::end() when process exits.
  Nb::end();

  return (int)ret;
}
