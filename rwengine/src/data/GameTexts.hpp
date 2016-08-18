#pragma once
#ifndef _GAMETEXTS_HPP_
#define _GAMETEXTS_HPP_
#include <string>
#include <unordered_map>

class GameTexts
{
  std::unordered_map<std::string, std::string> _textDB;

public:
  void addText(const std::string& id, const std::string& text) { _textDB.insert({id, text}); }

  std::string text(const std::string& id)
  {
    auto a = _textDB.find(id);
    if (a != _textDB.end()) {
      return a->second;
    }
    return id;
  }
};

#endif
