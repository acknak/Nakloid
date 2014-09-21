#ifndef WavParser_h
#define WavParser_h

#include <vector>
#include <boost/filesystem/path.hpp>

class WavHandler;

class WavParser {
 public:
  WavParser();
  WavParser(const boost::filesystem::path& path_input, WavHandler* const handler);
  virtual ~WavParser();

  bool isWavFile() const;
  bool parse();

  // accessor
  const boost::filesystem::path& getPathInput() const;
  void setPathInput(const boost::filesystem::path& path_input);
  void setWavHandler(const std::vector<WavHandler*>& handlers);
  void addWavHandler(WavHandler* const handler);

 private:
  WavParser(const WavParser& other);
  WavParser& operator=(const WavParser& other);

  boost::filesystem::path path_input;
  std::vector<WavHandler*> handlers;
};

#endif
