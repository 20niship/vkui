#include <algorithm>
#include <chrono>
#include <codecvt>
#include <fstream>
#include <locale>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>

#include "../../../include/icon.hpp"

int main() {
  const std::vector<std::string> list = {
    vkUI::Icon::ADDRESS_CARD_O,
    vkUI::Icon::ADDRESS_CARD,
    vkUI::Icon::ADDRESS_BOOK,
    vkUI::Icon::SEARCH,
    vkUI::Icon::SEARCH_MINUS,
    vkUI::Icon::SEARCH_PLUS,
    vkUI::Icon::FOLDER,
    vkUI::Icon::FOLDER_OPEN,
    vkUI::Icon::FILE,
    vkUI::Icon::ARCHIVE,
    vkUI::Icon::FILE_AUDIO_O,
    vkUI::Icon::VIDEO_CAMERA,
    vkUI::Icon::TEXT_HEIGHT,
    vkUI::Icon::TEXT_WIDTH,
    vkUI::Icon::BOLD,
    vkUI::Icon::BOLT,
    vkUI::Icon::ITALIC,
    vkUI::Icon::TWITTER,
    vkUI::Icon::SQUARE,
    vkUI::Icon::EXCLAMATION_TRIANGLE,
    vkUI::Icon::QUESTION,
    vkUI::Icon::BARS,
    vkUI::Icon::BATTERY_FULL,
    vkUI::Icon::BATTERY_EMPTY,
    vkUI::Icon::BATTERY_QUARTER,
    vkUI::Icon::BARCODE,
    vkUI::Icon::GIFT,
    vkUI::Icon::GIT,
    vkUI::Icon::ARROWS,
    vkUI::Icon::ARROWS_ALT,
    vkUI::Icon::ARROWS_H,
    vkUI::Icon::ARROWS_V,
    vkUI::Icon::ARROW_CIRCLE_DOWN,
    vkUI::Icon::ARROW_CIRCLE_LEFT,
    vkUI::Icon::ARROW_CIRCLE_RIGHT,
    vkUI::Icon::ARROW_CIRCLE_UP,
    vkUI::Icon::ARROW_LEFT,
    vkUI::Icon::ARROW_UP,
    vkUI::Icon::ARROW_RIGHT,
    vkUI::Icon::ARROW_DOWN,
    vkUI::Icon::PENCIL,
    vkUI::Icon::BITBUCKET,
    vkUI::Icon::PIE_CHART,
    vkUI::Icon::LINE_CHART,
    vkUI::Icon::ANGLE_DOWN,
    vkUI::Icon::ANGLE_RIGHT,
    vkUI::Icon::ANGLE_RIGHT,
    vkUI::Icon::ANGLE_UP,
    vkUI::Icon::ANCHOR,
    vkUI::Icon::COLUMNS,
    vkUI::Icon::ROAD,
    vkUI::Icon::ROCKET,
    vkUI::Icon::HAND_O_LEFT,
    vkUI::Icon::HAND_O_RIGHT,
    vkUI::Icon::HAND_O_UP,
    vkUI::Icon::HAND_O_DOWN,
    vkUI::Icon::HANDSHAKE_O,
    vkUI::Icon::LINK,
    vkUI::Icon::LEVEL_DOWN,
    vkUI::Icon::LEVEL_DOWN,
    vkUI::Icon::CLOCK_O,
    vkUI::Icon::CLOUD_DOWNLOAD,
    vkUI::Icon::CLOUD_UPLOAD,
    vkUI::Icon::CLIPBOARD,
    vkUI::Icon::CLONE,
    vkUI::Icon::CODE,
    vkUI::Icon::UNIVERSAL_ACCESS,
    vkUI::Icon::PAINT_BRUSH,
    vkUI::Icon::UNDO,
    vkUI::Icon::REDDIT,
    vkUI::Icon::DIAMOND,
    vkUI::Icon::QQ,
    vkUI::Icon::QUESTION_CIRCLE,
    vkUI::Icon::QUOTE_LEFT,
    vkUI::Icon::QUOTE_RIGHT,
    vkUI::Icon::DESKTOP,
    vkUI::Icon::ALIGN_CENTER,
    vkUI::Icon::ALIGN_JUSTIFY,
    vkUI::Icon::ALIGN_LEFT,
    vkUI::Icon::ALIGN_RIGHT,
    vkUI::Icon::MEDIUM,
    vkUI::Icon::MAXCDN,
    vkUI::Icon::MINUS,
    vkUI::Icon::MINUS_CIRCLE,
    vkUI::Icon::MINUS_SQUARE,
    vkUI::Icon::MOBILE,
    vkUI::Icon::MERCURY,
    vkUI::Icon::INBOX,
    vkUI::Icon::CIRCLE,
    vkUI::Icon::STAR,
    vkUI::Icon::STAR_HALF,
    vkUI::Icon::LINUX,
    vkUI::Icon::MAGIC,
    vkUI::Icon::WINDOW_CLOSE,
    vkUI::Icon::WINDOWS,
    vkUI::Icon::WINDOW_MAXIMIZE,
    vkUI::Icon::WINDOW_MINIMIZE,
    vkUI::Icon::PRINT,
    vkUI::Icon::PAPER_PLANE,
    vkUI::Icon::HASHTAG,
    vkUI::Icon::HAND_POINTER_O,
    vkUI::Icon::HANDSHAKE_O,
    vkUI::Icon::BARS,
    vkUI::Icon::CUBE,
    vkUI::Icon::CUBES,
    vkUI::Icon::DEAF,
    vkUI::Icon::VOLUME_UP,
    vkUI::Icon::VOLUME_CONTROL_PHONE,
    vkUI::Icon::VOLUME_DOWN,
    vkUI::Icon::VOLUME_OFF,
    vkUI::Icon::MONEY,
    vkUI::Icon::ADJUST,
    vkUI::Icon::LOW_VISION,
    vkUI::Icon::EYE,
    vkUI::Icon::EYE_SLASH,
    vkUI::Icon::EYEDROPPER,
  };

  const std::string output_filename = "../Glyphs/icon.csv";
  std::ofstream ofs(output_filename);
  if(!ofs) {
    std::cout << "Could not open the output file" << std::endl;
    return -1;
  }
  int idx = 0;
  for(const auto i : list) {
    std::u32string u32str = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(i);
    std::cout << list[idx] << " = ";
    std::cout << std::hex << static_cast<int>(u32str[0]) << std::endl;
    ofs << std::to_string(u32str[0]) << ",";
    idx++;
  }
}
