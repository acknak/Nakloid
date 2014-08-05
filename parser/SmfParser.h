#ifndef SmfParser_h
#define SmfParser_h

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem/fstream.hpp>
#include "SmfHandler.h"

class SmfParser {
 public:
  SmfParser(){}
  SmfParser(const boost::filesystem::path& path_input, SmfHandler* const handler);
  virtual ~SmfParser(){}

  bool isSmfFile() const;
  bool parse();

  // accessor
  const boost::filesystem::path& getInputPath() const;
  void setInputPath(const boost::filesystem::path& path_input);
  const std::vector<SmfHandler*>& getSmfHandler() const;
  void setSmfHandler(const std::vector<SmfHandler*>& handlers);
  void addSmfHandler(SmfHandler* const handler);

 private:
  SmfParser(const SmfParser& other);
  SmfParser& operator=(const SmfParser& other);

  boost::filesystem::path path_input;
  std::vector<SmfHandler*> handlers;
  static unsigned const char mthd[]; //smf header
  static unsigned const char mtrk[]; //data chunk header
  static unsigned const char eot[]; //data chunk end
};

#endif

