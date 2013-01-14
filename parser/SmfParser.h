#ifndef SmfParser_h
#define SmfParser_h

#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include "SmfHandler.h"

// Reference Object
class SmfParser {
 public:
  SmfParser();
  explicit SmfParser(std::string filename);
  explicit SmfParser(SmfHandler* handler);
  SmfParser(std::string filename, SmfHandler* handler);
  ~SmfParser();

  bool isSmfFile();
  bool parse();

  // accessor
  std::string getInput();
  void setInput(std::string filename);
  std::vector<SmfHandler*> getSmfHandler();
  void setSmfHandler(std::vector<SmfHandler*> handlers);
  void addSmfHandler(SmfHandler* handler);

 private:
  SmfParser(const SmfParser& other);
  SmfParser& operator=(const SmfParser& other);

  std::string input;
  std::vector<SmfHandler*> handlers;
  static unsigned const char mthd[]; //smf header
  static unsigned const char mtrk[]; //data chunk header
  static unsigned const char eot[]; //data chunk end
};

#endif

