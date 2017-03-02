#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "optionparser.h"

#include "libAlazarAPI.h"
#include "logger.h"

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

enum optionIndex {
  UNKNOWN,
  HELP,
  MODE,
  SEGMENTS,
  WAVEFORMS,
  ROUNDROBINS,
  RECORDLENGTH,
  BUFFER,
  SAMPLINGRATE,
  TIMEOUT,
  LOGGING_LEVEL
};
const option::Descriptor usage[] = {
    {UNKNOWN, 0, "", "", option::Arg::None, "USAGE: apiExample [options]\n\n"
                                            "Options:"},
    {HELP, 0, "", "help", option::Arg::None,
     "	--help	\tPrint usage and exit."},
    {MODE, 0, "", "mode", option::Arg::None,
     "  --mode\tAcquisition mode, 'digitizer' or 'averager' (default)"},
    {SEGMENTS, 0, "", "segments", option::Arg::Numeric,
     "  --segments\tNumber of segments (default = 1)"},
    {WAVEFORMS, 0, "", "waveforms", option::Arg::Numeric,
     "  --waveforms\tNumber of waveforms (default = 1)"},
    {ROUNDROBINS, 0, "", "roundrobins", option::Arg::Numeric,
     "  --roundrobins\tNumber of round robins (default = 1)"},
    {RECORDLENGTH, 0, "", "samples", option::Arg::Numeric,
     "  --samples\tNumber of samples (default = 4096)"},
    {BUFFER, 0, "", "buffer", option::Arg::Numeric,
     "  --buffer\tBuffer size (default = 4096 * 2)"},
    {SAMPLINGRATE, 0, "", "samplingRate", option::Arg::Numeric,
     "  --samplingRate\tSampling rate (default = 500e6)"},
    {TIMEOUT, 0, "", "timeout", option::Arg::Numeric,
     "  --timeout\tTimeout in milliseconds (default = 1000)"},
    {LOGGING_LEVEL, 0, "", "logLevel", option::Arg::Numeric,
     "	--logLevel	\t(optional) Logging level level to print to console "
     "(optional; default=2/INFO)."},
    {UNKNOWN, 0, "", "", option::Arg::None,
     "\nExamples:\n"
     "\tapiExample --samples=2048\n"
     "\tapiExample --samples=2048 --segments=10\n"},
    {0, 0, 0, 0, 0, 0}};

int main(int argc, char *argv[]) {

  argc -= (argc > 0);
  argv += (argc > 0); // skip program name argv[0] if present
  option::Stats stats(usage, argc, argv);
  option::Option *options = new option::Option[stats.options_max];
  option::Option *buffer = new option::Option[stats.buffer_max];
  option::Parser parse(usage, argc, argv, options, buffer);

  if (parse.error())
    return -1;

  if (options[HELP] || argc == 0) {
    option::printUsage(std::cout, usage);
    return 0;
  }

  for (option::Option *opt = options[UNKNOWN]; opt; opt = opt->next())
    std::cout << "Unknown option: " << opt->name << "\n";

  for (int i = 0; i < parse.nonOptionsCount(); ++i)
    std::cout << "Non-option #" << i << ": " << parse.nonOption(i) << "\n";

  // Logging level
  // TLogLevel logLevel = logINFO;
  // if (options[LOGGING_LEVEL]) {
  //   logLevel = TLogLevel(atoi(options[LOGGING_LEVEL].arg));
  // }

  // todo - make more parameters user configurable
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
  };

  if (options[MODE]) {
    config.acquireMode = options[MODE].arg;
  }
  if (options[RECORDLENGTH]) {
    config.recordLength = atoi(options[RECORDLENGTH].arg);
  }
  if (options[SEGMENTS]) {
    config.nbrSegments = atoi(options[SEGMENTS].arg);
  }
  if (options[WAVEFORMS]) {
    config.nbrWaveforms = atoi(options[WAVEFORMS].arg);
  }
  if (options[ROUNDROBINS]) {
    config.nbrRoundRobins = atoi(options[ROUNDROBINS].arg);
  }
  if (options[SAMPLINGRATE]) {
    config.samplingRate = atof(options[SAMPLINGRATE].arg);
  }
  uint32_t timeout = 1000;
  if (options[TIMEOUT]) {
    timeout = atoi(options[TIMEOUT].arg);
  }

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
  uint32_t buffer_timeout = 0;
  while (count < acqParams.numberAcquisitions) {
    // printf("rr %d count %d\n",config.nbrRoundRobins,count);
    force_trigger(1);
    fflush(stdout);
    if( wait_for_acquisition(1,ch1, ch2) ) {
      buffer_timeout=0;
      count++;
      fwrite(ch1,sizeof(float),acqParams.samplesPerAcquisition,f1);
      fwrite(ch2,sizeof(float),acqParams.samplesPerAcquisition,f2);
    }
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
    if (buffer_timeout++ > timeout)
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
