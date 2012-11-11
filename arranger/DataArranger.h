#ifndef DataArranger_h
#define DataArranger_h

#include <vector>
#include <cmath>

class DataArranger {
 public:
  const static short edge_back_ms;
  const static short edge_front_ms;

  static void edge_back(std::vector<short> *data, long sps);
  static void edge_front(std::vector<short> *data, long sps);

 private:
  static double ms2pos(long ms, long sps);
};

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
