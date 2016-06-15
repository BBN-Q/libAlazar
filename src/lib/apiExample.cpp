#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <unistd.h>

#include "libAlazarAPI.h"
#include "libAlazarConfig.h"
#include "logger.h"

using namespace boost::program_options;

void waitForQuit(void) {
  std::string line;

  std::string strq("q");
  while (1) {
    getline(std::cin, line);
    std::cout << line << std::endl;
    if (!line.compare(strq)) {
      printf("Exiting ...\n");
      exit(0);
    }
  }
  return;
}

int main(int argc, char *argv[]) {

  variables_map vm;
  try {
    options_description desc{"Options"};
    desc.add_options()("help,h", "Help screen")(
        "mode", value<std::string>()->default_value("averager"),
        "acquire Mode")("segments", value<uint32_t>()->default_value(1),
                        "number of segments")(
        "waveforms", value<uint32_t>()->default_value(1),
        "number of waveforms")("roundrobins",
                               value<uint32_t>()->default_value(1),
                               "number of roundrobins")(
        "recordlength", value<uint32_t>()->default_value(4096),
        "record length")("buffer", value<uint32_t>()->default_value(4096 * 2),
                         "buffer size")(
        "samplingRate", value<float>()->default_value(500e6), "sample rate")(
        "timeout", value<uint32_t>()->default_value(1000), "timeout in ms");

    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help")) {
      std::cout << desc << '\n';
      exit(0);
    }

  } catch (const error &ex) {
    std::cerr << ex.what() << '\n';
  }

  // todo - make parameters user configurable
  ConfigData_t config = {
      "averager", // acquire mode - "digitizer" or "averager"
      "Full",     // bandwidth - "Full" or "20MHz"
      "ref",      // todo - clockType, parameter currently not used
      0.0,        // trigger delay in seconds
      true,       // instrumrnt enabled
      "myAlazar", // instrument label
      4096,     // segmentLength - must be greater than 256 and a multiple of 16
      1,        // number of segments
      1,        // number of waveforms
      1,        // number of round robins
      500e6,    // sample rate
      "DC",     // trigger coupling - "AC" or "DC"
      1000,     // trigger threshold in mV
      "rising", // trigger edge slope - "rising" or "falling"
      "Ext",    // trigger source -  only "Ext" supported
      "AC",     // channel coupling - "AC" or "DC"
      0.0,      // channel offset
      4.0,      // channel scale
      4096 * 2, // max buffer size
  };

  config.acquireMode = vm["mode"].as<std::string>().c_str();
  config.recordLength = vm["recordlength"].as<uint32_t>();
  config.nbrSegments = vm["segments"].as<uint32_t>();
  config.nbrWaveforms = vm["waveforms"].as<uint32_t>();
  config.nbrRoundRobins = vm["roundrobins"].as<uint32_t>();
  config.bufferSize = vm["buffer"].as<uint32_t>();
  config.samplingRate = vm["samplingRate"].as<float>();

  AcquisitionParams_t acqParams;

  // this is a hack because because MSYS2 does not handle
  std::thread quit(waitForQuit);
  quit.detach();

  connectBoard(1, NULL);
  if (setAll(1, &config, &acqParams) < 0) {
    exit(-1);
  }

  // allocate channel data memory
  float *ch1 = new float[acqParams.samplesPerAcquisition];
  float *ch2 = new float[acqParams.samplesPerAcquisition];

  // open a couple of files for storing the data
  FILE *f1 = fopen("ch1.dat", "wb");
  FILE *f2 = fopen("ch2.dat", "wb");

  acquire(1);

#if 1
  uint32_t count = 0;
  uint32_t timeout = 0;
  while (count < acqParams.numberAcquistions) {
    // printf("rr %d count %d\n",config.nbrRoundRobins,count);
        force_trigger(1);
        fflush(stdout);
        if( wait_for_acquisition(1,ch1, ch2) )
        {
            timeout=0;
            count++;
            fwrite(ch1,sizeof(float),acqParams.samplesPerAcquisition,f1);
            fwrite(ch2,sizeof(float),acqParams.samplesPerAcquisition,f2);
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
    if (timeout++ > vm["timeout"].as<uint32_t>())
      break;
  }
#endif
  stop(1);
  disconnect(1);

  delete[] ch1;
  delete[] ch2;
  fclose(f1);
  fclose(f2);
}
