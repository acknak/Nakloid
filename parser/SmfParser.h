#ifndef SmfParser_h
#define SmfParser_h

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "SmfHandler.h"

class SmfParser {
 public:
  SmfParser();
  explicit SmfParser(const std::wstring& filename);
  explicit SmfParser(SmfHandler* const handler);
  SmfParser(const std::wstring& filename, SmfHandler* const handler);
  virtual ~SmfParser();

  bool isSmfFile() const;
  bool parse();

  // accessor
  const std::wstring& getInput() const;
  void setInput(const std::wstring& filename);
  const std::vector<SmfHandler*>& getSmfHandler() const;
  void setSmfHandler(const std::vector<SmfHandler*>& handlers);
  void addSmfHandler(SmfHandler* const handler);

 private:
  SmfParser(const SmfParser& other);
  SmfParser& operator=(const SmfParser& other);

  std::wstring input;
  std::vector<SmfHandler*> handlers;
  static unsigned const char mthd[]; //smf header
  static unsigned const char mtrk[]; //data chunk header
  static unsigned const char eot[]; //data chunk end
};

#endif

