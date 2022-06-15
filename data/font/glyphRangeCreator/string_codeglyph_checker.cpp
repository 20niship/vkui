
#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>


int main(){
  std::string a = "\\\"'~=|@[]?<>+-*/{}";
  std::u32string u32str  = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(a);
  for(int i=0; i<u32str.size(); i++){
    const auto s = u32str[i];
    std::cout << a.at(i) << " = " << std::hex << (int)s << std::endl;
  }
  return 0;
}

