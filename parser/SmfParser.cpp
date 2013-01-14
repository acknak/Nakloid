#include "SmfParser.h"

using namespace std;

const unsigned char SmfParser::mthd[] = {'M','T','h','d',0,0,0,6,0,1}; //smf header
const unsigned char SmfParser::mtrk[] = {'M','T','r','k'}; //data chunk header
const unsigned char SmfParser::eot[] = {0xFF, 0x2F, 0}; //data chunk end

SmfParser::SmfParser(){}

SmfParser::~SmfParser(){}

SmfParser::SmfParser(string filename)
{
  setInput(filename);
}

SmfParser::SmfParser(SmfHandler* handler)
{
  addSmfHandler(handler);
}

SmfParser::SmfParser(string filename, SmfHandler* handler)
{
  setInput(filename);
  addSmfHandler(handler);
}

bool SmfParser::isSmfFile()
{
  if (input.empty()) {
    cerr << "[SmfParser::isSmfFile] input is NULL" << endl;
    return false;
  }

  ifstream ifs(input.c_str(), ios::in | ios::binary);
  if (!ifs) {
    cerr << "[SmfParser::isSmfFile] file '" << input << "' cannot open" << endl;
    return false;
  }

  unsigned char data;
  for (int i=0; i<10; i++) {
    ifs.read((char*)&data, sizeof(char));
    if (data != mthd[i]) {
      cerr << "[SmfParser::isSmfFile] file '" << input << "' is not SMF(format 1)" << endl;
      return false;
    } 
  }

  return true;
}

bool SmfParser::parse()
{
  if (!isSmfFile())
    return false;

  ifstream ifs(input.c_str(), ios::in | ios::binary);
  ifs.seekg(sizeof(char)*10); //skip smf header
  unsigned char data;
  vector<SmfHandler*>::iterator it;

  // get number of track
  short num_track = 0;
  ifs.read((char*)&data, sizeof(char));
  num_track = (int)data << 8;
  ifs.read((char*)&data, sizeof(char));
  num_track += (int)data;

  // get timebase
  short timebase = 0;
  ifs.read((char*)&data, sizeof(char));
  timebase = (int)data << 8;
  ifs.read((char*)&data, sizeof(char));
  timebase += (int)data;

  // set info
  for (it=handlers.begin(); it!=handlers.end(); it++)
    (*it)->smfInfo(num_track, timebase);

  // parse data chunk
  cout << "num_track:" << num_track << endl;
  for (int i=0; i<num_track; i++) {
    for (it=handlers.begin(); it!=handlers.end(); it++)
      (*it)->trackChange(i);

    // get chunk head
    for (int j=0; j<4; j++) {
      ifs.read((char*)&data, sizeof(char));
      if (data != mtrk[j]) {
        cerr << "[SmfParser::parse] " << i << "th MTrk cannot find" << endl;
        return false;
      }
    }

    // get chunk data size
    long datasize = 0;
    for (int j=0; j<4; j++) {
      ifs.read((char*)&data, sizeof(char));
      datasize <<= 8;
      datasize += (long)data;
    }
    cout << i << "th chunk data size: " << datasize << endl;

    // get chunk data
    unsigned char status = 0;
    for (long j=0; j<datasize; j++) {
      // get deltatime
      long deltatime = 0;
      do {
        ifs.read((char*)&data, sizeof(char));
        deltatime <<= 7;
        deltatime += (long)(data & 0x7F);
      } while (data & 0x80);

      // get msg
      ifs.read((char*)&data, sizeof(char));
      if (data >= 0x80)
        status = data; //status byte
      else
        ifs.putback(data); //running status

      switch (status) {
      case 0xFF: //meta event
        {
          ifs.read((char*)&data, sizeof(char));
          unsigned char type = data;
          long meta_datasize = 0;
          do {
            ifs.read((char*)&data, sizeof(char));
            meta_datasize <<= 7;
            meta_datasize += (int)(data & 0x7F);
          } while (data & 0x80);
          unsigned char *meta_data = new unsigned char[meta_datasize];
          ifs.read((char*)meta_data, (sizeof(unsigned char))*meta_datasize);
          for (it=handlers.begin(); it!=handlers.end(); it++)
            (*it)->eventMeta(deltatime, type, meta_datasize, meta_data);
          delete [] meta_data;

          if (type == 0x2F) //chunk end
            j = datasize;
          break;
        }
      case 0xF0:
      case 0xF7: //system exclusive event
        {
          long sys_ex_datasize = 0;
          do {
            ifs.read((char*)&data, sizeof(char));
            sys_ex_datasize <<= 7;
            sys_ex_datasize += (int)(data & 0x7F);
          } while (data & 0x80);
          unsigned char *sys_ex_data = new unsigned char[sys_ex_datasize];
          ifs.read((char*)sys_ex_data, sizeof(char)*sys_ex_datasize);
          for (it=handlers.begin(); it!=handlers.end(); it++)
            (*it)->eventSysEx(deltatime, sys_ex_datasize, sys_ex_data);
          delete [] sys_ex_data;
          break;
        }
      default: //midi event
        {
          unsigned char msgdata[2] = {0,0};
          if (status!=0xF6 && status!=0xF7) {
            ifs.read((char*)&data, sizeof(char));
            msgdata[0] = data;
            if (((status&0xF0) != 0xC0) && ((status&0xF0) != 0xD0) && status!=0xF1 && status!=0xF3) {
              ifs.read((char*)&data, sizeof(char));
              msgdata[1] = data;
            }
          }
          for (it=handlers.begin(); it!=handlers.end(); it++)
            (*it)->eventMidi(deltatime, status, msgdata);
          break;
        }
      }
    }
  }

  cout << "convert finished"<< endl;

  return true;
}

string SmfParser::getInput()
{
  return input;
}

void SmfParser::setInput(string filename)
{
  input = filename;
}

void SmfParser::setSmfHandler(vector<SmfHandler*> handlers)
{
  handlers.clear();
  this->handlers = handlers;
}

void SmfParser::addSmfHandler(SmfHandler* handler)
{
  handlers.push_back(handler);
}
