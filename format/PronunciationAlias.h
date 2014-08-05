#ifndef PronunciationAlias_h
#define PronunciationAlias_h

#include <map>
#include <string>
#include <boost/assign.hpp>

class PronunciationAlias {
 public:
  PronunciationAlias():prefix(L""),pron(L""),suffix(L""){}
  explicit PronunciationAlias(const std::wstring& alias);
  PronunciationAlias(const std::wstring& prefix, const std::wstring& pron, const std::wstring& suffix):prefix(prefix),pron(pron),suffix(suffix){}
  bool operator==(const PronunciationAlias& other) const;
  bool operator!=(const PronunciationAlias& other) const;

  std::wstring prefix;
  std::wstring pron;
  std::wstring suffix;

  bool checkVCV() const;
  std::wstring getAliasString() const;
  std::wstring getVowel() const;
  std::wstring getPrefixVowel() const;
  static bool isVowel(std::wstring vowel);
  static std::wstring vow2pron(std::wstring vowel);
  static bool isPron(std::wstring pron);
  static std::wstring pron2vowel(std::wstring pron);

 private:
  static std::map<std::wstring, std::wstring> data_vowel2pron;
  static std::map<std::wstring, std::wstring> data_pron2vowel;
};

#endif
