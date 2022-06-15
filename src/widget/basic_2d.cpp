#include <cstdlib>
#include <engine.hpp>
#include <glm/detail/qualifier.hpp>
#include <string>
#include <type_traits>
#include <widgets/basic_2d.hpp>
#include <algorithm>
#include <vector>
#include <logger.hpp>

namespace vkUI{

uiRootWidget2D::uiRootWidget2D(){}
void uiRootWidget2D::render(){ for(auto w : widgets){ w->render(); }}
void uiRootWidget2D::calcHoveringWidget(int x, int y){
    uiWidget *LastHovering = HoveringWidget; 
    if(!LastHovering && LastHovering->contains(x,y) && LastHovering->isActive() && LastHovering != this) return;
    uiWidget *w{this};
    bool search_all = false;
    while(w->widgets.size() > 0 && !search_all){
        const auto w_start = w;
        for(int i=w->widgets.size()-1; i>=0; i--){
            const auto w2 = w->widgets[i];
            if(w2->contains(x, y) && w2->isActive()){ w = w2; break;}
        }
        if(w_start == w){ break; }
    }
    if(LastHovering != w){
        LastHovering->setHoveringFlag(false);
        w->setHoveringFlag(true);
        HoveringWidget = w;
    }
}

bool uiRootWidget2D::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1,[ [maybe_unused]] int num_1, [[maybe_unused]] int num_2, [[maybe_unused]] const char **strings){ 
    double xpos, ypos;
    glfwGetCursorPos(Engine::getDrawingWindow()->getGLFWwindow(), &xpos, &ypos);
    // 一次的に機能停止
    // if(CallbackResizer(flag, {xpos, ypos}, num_1, num_2, strings)) return true;

    switch (flag){
    case uiCallbackFlags::OnHover:break;
    case uiCallbackFlags::OffHover:break;
    case uiCallbackFlags::ValueChange:break;
    case uiCallbackFlags::CharInput:break;
    case uiCallbackFlags::ShouldClose:break;

    case uiCallbackFlags::ResizeTo:
        size = vec2_1;
        // needRendering(true);
        // needApplyAlignment();
        // needCalcInnerSize();
        break;

    case uiCallbackFlags::MouseMove:
        calcHoveringWidget(xpos, ypos);
        if(HoveringWidget != static_cast<uiWidget *>(this)){
            HoveringWidget->CallbackFunc(uiCallbackFlags::MouseMove, {(int)xpos, (int)ypos}, HoveringWidget == FocusedWidget, num_2, nullptr); }
        if(FocusedWidget !=  static_cast<uiWidget *>(this) && HoveringWidget != FocusedWidget){ 
            FocusedWidget->CallbackFunc(uiCallbackFlags::MouseMove, {(int)xpos, (int)ypos}, false, num_2, nullptr); }
        break;

    case uiCallbackFlags::LMouseDown:
    case uiCallbackFlags::RMouseDown:
    case uiCallbackFlags::CMouseDown:
        calcHoveringWidget(xpos, ypos);
        if(FocusedWidget != HoveringWidget){
            FocusedWidget->setFocusedFlag(false);
            FocusedWidget = HoveringWidget;
            FocusedWidget->setFocusedFlag(true);
        }
        if(FocusedWidget != this) {
            FocusedWidget->CallbackFunc(flag, {(int)xpos, (int)ypos}, num_1, num_2, strings);
        }
        break;

    case uiCallbackFlags::LMouseUP:
    case uiCallbackFlags::RMouseUP:
    case uiCallbackFlags::CMouseUP:
        calcHoveringWidget(xpos, ypos);
        if(FocusedWidget  != this) FocusedWidget->CallbackFunc(flag, {(int)xpos, (int)ypos}, num_1, num_2, strings);
        break;

    case uiCallbackFlags::MouseScroll:
        calcHoveringWidget(xpos, ypos);
        if(HoveringWidget != this){
          const auto resolved = HoveringWidget->CallbackFunc(uiCallbackFlags::MouseScroll, {(int)xpos, (int)ypos}, num_1, num_2, strings);
          if(resolved && FocusedWidget != HoveringWidget){
            FocusedWidget->setFocusedFlag(false);
            FocusedWidget = HoveringWidget;
            FocusedWidget->setFocusedFlag(true);
          }
        }
        if(FocusedWidget  != this) FocusedWidget->CallbackFunc(uiCallbackFlags::MouseScroll, {(int)xpos, (int)ypos}, num_1, num_2, strings);
        break;

    // case uiCallbackFlags::DragDrop:
    //     calcHoveringWidget(xpos, ypos);
    //     FocusedWidget->setFocusedFlag(false);
    //     FocusedWidget = HoveringWidget;
    //     FocusedWidget->setFocusedFlag(true);
    //     if(FocusedWidget != this) FocusedWidget->CallbackFunc(uiCallbackFlags::DragDrop, Vector2d(xpos, ypos),num_1, num_2, strings);
    //     printf("drop %d\n", num_1);
    //     for (int i = 0; i < num_1; i++) printf("%s\n", strings[i]);
    //     break;

    case uiCallbackFlags::Keyboard:
        calcHoveringWidget(xpos, ypos);
        FocusedWidget->setFocusedFlag(false);
        FocusedWidget->setFocusedFlag(true);
        if(FocusedWidget != this) FocusedWidget->CallbackFunc(uiCallbackFlags::Keyboard, {(int)xpos, (int)ypos}, num_1, num_2, strings);
        break;


        // switch(num_1){
        //     case GLFW_KEY_F11:
        //         if(num_2 == GLFW_PRESS) setFullScreen(!wndStyle.isFullScreen);
        //         break;
        //     case GLFW_KEY_ESCAPE:
        //         if(wndStyle.isFullScreen && num_2 == GLFW_PRESS) setFullScreen(false);
        //         break;
        // }

        break;
default:
    uiLOGE << "Not set flag!!\n";
    disp((uint16_t)flag);

    break;
}

return true;
}

// -----------------------------------------------------
//   [SECTION]   Widget Text
// -----------------------------------------------------
uiLabel::uiLabel(const std::string text_){text = text_; col={255,255,255}; }
uiLabel::uiLabel(const std::string text_, const Vector3b &col_){ text = text_; col = col_;} 
void uiLabel::render() {
    auto wnd = Engine::getDrawingWindow();
	if(!flags.Active){
		needRendering(false);
		std::cout << "Not visivle or not active widget\n";
		return;
	}
	if(flags.needRendering){
        const auto s = wnd->AddString2D(text, pos,1, wnd->getClippingRect(), col);
        if(s != outerSize){outerSize = s; impl_needCalcAlinment_parent(); }
		needRendering(false);
	}
}

bool uiLabel::CallbackFunc([[maybe_unused]]uiCallbackFlags flag, [[maybe_unused]]Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings){
	return true;
}


uiButton::uiButton(std::string text_, bool *value_){ value = value_; last_value = !(*value); text = text_; }
void uiButton::render() {
	if(!flags.Active){
		needRendering(false);
		std::cout << "Not visivle or not active widget\n";
		return;
	}

	if(*value != last_value){needRendering(true);}
	if(flags.needRendering){
        auto wnd = Engine::getDrawingWindow();
		const auto style = Engine::getStyle();
		if(*value){
			wnd->AddRectPosSize_clip(pos, size, style->col_WidgetBg);
		}else{
            wnd->AddRectPosSize_clip(pos, size, flags.isHovering ? (style->col_WidgetBgHover): (style->col_WidgetBgSelected));
		}
		wnd->AddRectPosSize_clip(pos, size, style->col_WidgetLine, 1);
		const auto s = wnd->AddString2D(text, pos + style->WidgetPadding, 1, wnd->getClippingRect()) + style->WidgetPadding * 2;
        if(s != outerSize){ impl_needCalcAlinment_parent(); outerSize = s; }
		last_value = *value;
		needRendering(false);
	}
}

bool uiButton::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings){
	if ( flag == uiCallbackFlags::LMouseDown && contains(vec2_1[0], vec2_1[1])){
	// if ( flag == uiCallbackFlags::LMouseDown){
		*value = !(*value);
		needRendering(true);
	}
	return true;
}

uiButtonFunc::uiButtonFunc(const std::string text_, std::function<void(void)> func_){ text = text_; func = func_; needRendering(true); }
void uiButtonFunc::render() {
	if(!flags.Active){ needRendering(false); return; }
	if(flags.needRendering){
    auto wnd = Engine::getDrawingWindow();
		const auto style = Engine::getStyle();
		wnd->AddRectPosSize_clip(pos, size, style->col_WidgetBg);
		wnd->AddRectPosSize_clip(pos, size, style->col_WidgetLine, 1);
		const auto s = wnd->AddString2D(text, pos + style->WidgetPadding, 1, wnd->getClippingRect()) + style->WidgetPadding * 2;
    if(s != outerSize){ impl_needCalcAlinment_parent(); outerSize = s; }
		needRendering(false);
	}
}

bool uiButtonFunc::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings){
	if ( flag == uiCallbackFlags::LMouseDown && contains(vec2_1[0], vec2_1[1])){
    func();
		needRendering(true);
	}
	return true;
}
uiCheckbox::uiCheckbox(std::string text_, bool *value_){ value = value_; last_value = !(*value); text = text_; }
void uiCheckbox::render() {
	if(!flags.Active){
		needRendering(false);
		std::cout << "Not visivle or not active widget\n";
		return;
	}
	if(*value != last_value){needRendering(true);}
	if(! flags.needRendering) return;
    
    auto wnd = Engine::getDrawingWindow();
    const auto style = Engine::getStyle();
    // 1. draw checkbox
    constexpr int  checkbox_size = 14;
    const int ypad = (size[1] - checkbox_size)/2;
    wnd->AddRectPosSize_clip(pos + Vector2d(0,ypad), {checkbox_size, checkbox_size}, style->col_WidgetBg);
    wnd->AddRectPosSize_clip(pos+ Vector2d(0,ypad), {checkbox_size, checkbox_size}, style->col_WidgetLine, 1);
    if(*value) {wnd->AddCheck2D(pos+ Vector2d(0,ypad), checkbox_size, {255,255,255}, 2);}
    const auto s = wnd->AddString2D(text, pos + style->WidgetPadding + Vector2d{checkbox_size, 0}, 1, wnd->getClippingRect()) + style->WidgetPadding * 2;
    if(s != outerSize){ impl_needCalcAlinment_parent(); outerSize = s; }
    last_value = *value;
    needRendering(false);
}

bool uiCheckbox::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1,[ [maybe_unused]] int num_1, [[maybe_unused]] int num_2, [[maybe_unused]] const char **strings){
	if ( flag == uiCallbackFlags::LMouseDown && contains(vec2_1[0], vec2_1[1])){
		*value = !(*value); needRendering(true); return true;
    }
    return false;
}

template <typename T>
uiSliderH<T>::uiSliderH(const std::string &text_, T *value_, const Vector2 &range_, const double delta_){
    text = text_;
    value = value_;
    range = range_;
    delta = delta_;
	last_value = *value_;
	flags.EnableAutoExpandX = true;
	flags.EnableAutoShrinkX = true;
}
template <typename T>
void uiSliderH<T>::render() {
	if(!flags.Active){ needRendering(false); return; }
    auto wnd = Engine::getDrawingWindow();
	const auto style = Engine::getStyle();
	
	if((*value != last_value) || flags.needRendering){
		int pos_x = pos[0] + size[0] * (*value - range[0])/(range[1] - range[0]);
		wnd->AddRectPosSize(pos, size, style->col_WidgetBg);
		wnd->AddRectPosSize(pos, size, style->col_WidgetLine);
		wnd->AddRectPosSize({pos_x, pos[1]}, {10, size[1]}, {200, 55, 200 });
		char tt[50];
		sprintf (tt, "%s : %0.5f", text.c_str(), (float)(*value)); // typename Tがintのときとかは%fではなく%dでしょ。あと桁数指定しようよ
		const auto s =wnd->AddString2D(std::string(tt), pos+style->WidgetPadding, 1) + style->WidgetPadding*2;
		last_value = *value;
    if(s != outerSize){ impl_needCalcAlinment_parent(); outerSize = s; }
		needRendering(false);
	}
}
template<typename T>
bool uiSliderH<T>::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1,[ [maybe_unused]] int num_1, [[maybe_unused]] int num_2, [[maybe_unused]] const char **strings){
  T tmp_value = float((vec2_1[0] - pos[0])*(range[1] - range[0]))/float(size[0]) + range[0];
	if ( flag == uiCallbackFlags::LMouseDown){
		*value = tmp_value; 
	}else if(flag ==uiCallbackFlags::MouseScroll){
		*value += num_2 > 0 ? delta : -delta;
	}else if(flag ==uiCallbackFlags::MouseMove){
		if (num_2 & 0b01){ *value = tmp_value; }  
	}
	*value = std::clamp<float>(*value, range[0], range[1]);
	return true;
}

// the explicit instanciation
template uiSliderH<double>::uiSliderH(const std::string&, double *, const Vector2&, const double);
template uiSliderH<float>::uiSliderH(const std::string&, float*, const Vector2&, const double);
template uiSliderH<int>::uiSliderH(const std::string&, int*, const Vector2&, const double);


template <typename T>
uiRange<T>::uiRange(const std::string &text_, _Vec<T,2> *value_, const Vector2 &range_, const double delta_){
    text = text_;
    value = value_;
    range = range_;
    delta = delta_;
	last_value = *value_;
	flags.EnableAutoExpandX = true;
	flags.EnableAutoShrinkX = true;
}
template <typename T>
void uiRange<T>::render() {
	if(!flags.Active){ needRendering(false); return; }
    auto wnd = Engine::getDrawingWindow();
	const auto style = Engine::getStyle();
	if((*value != last_value) || flags.needRendering){
		wnd->AddRectPosSize(pos, size, style->col_WidgetBg);
		wnd->AddRectPosSize(pos, size, style->col_WidgetLine);

		const int p0 = pos[0] + size[0] * ((*value)[0] - range[0])/(range[1] - range[0]);
		const int p1 = pos[0] + size[0] * ((*value)[1] - range[0])/(range[1] - range[0]);
		wnd->AddRectPosSize({p0, pos[1]}, {p1-p0, size[1]}, {200,55,200 });
        wnd->AddLine2D({p0, pos[1]}, {p0, pos[1]+size[1]}, {255,200,200},1);
        wnd->AddLine2D({p1, pos[1]}, {p1, pos[1]+size[1]}, {255,200,200},1);

		char tt[70];
		sprintf (tt, "%s:%2.f - %2.f", text.c_str(), (float)(*value)[0], (float)(*value)[1]);
		const auto s =wnd->AddString2D(std::string(tt), pos+style->WidgetPadding, 1) + style->WidgetPadding*2;
		last_value = *value;
    if(s != outerSize){ impl_needCalcAlinment_parent(); outerSize = s; }
		needRendering(false);
	}
}
template<typename T>
bool uiRange<T>::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, [[maybe_unused]] int num_1, [[maybe_unused]] int num_2, [[maybe_unused]] const char **strings){
    // selecting_idx = 0 : first,   selecting_idx = 1 : second
    const int p0 = pos[0] + size[0] * ((*value)[0] - range[0])/(range[1] - range[0]);
    const int p1 = pos[0] + size[0] * ((*value)[1] - range[0])/(range[1] - range[0]);
    if ( flag == uiCallbackFlags::LMouseDown || flag ==uiCallbackFlags::MouseScroll){
      const int th = std::abs(p1-p0)/3;
        if(std::abs(vec2_1[0]-p0) < th) selecting_idx = 0; 
        else if(std::abs(vec2_1[0]-p1) < th) selecting_idx = 1; 
        else selecting_idx = -1;
    }
    if(selecting_idx < 0) return false; 
    if(Engine::getFocusedWidget() != this) return false;

    if(flag ==uiCallbackFlags::MouseScroll){
        (*value)[selecting_idx] += num_2 > 0 ? delta : -delta;
        return true;
    }else if(flag ==uiCallbackFlags::MouseMove && (num_2 & 0b01) && selecting_idx >= 0){
        (*value)[selecting_idx] = range[0] + (vec2_1[0] - pos[0]) * (range[1] - range[0]) / size[0];
    }
	
	(*value)[0] = std::clamp<T>((*value)[0], range[0], range[1]);
	(*value)[1] = std::clamp<T>((*value)[1], range[0], range[1]);
	return true;
}

// the explicit instanciation
template uiRange<double>::uiRange(const std::string&, _Vec<double, 2> *, const Vector2&, const double);
template uiRange<float>::uiRange(const std::string&, _Vec<float, 2> *, const Vector2&, const double);
template uiRange<int>::uiRange(const std::string&, _Vec<int, 2> *, const Vector2&, const double);
template uiRange<int64_t>::uiRange(const std::string&, _Vec<int64_t, 2> *, const Vector2&, const double);

uiMultiLine::uiMultiLine(const std::string text_){
  text = text_;
  flags.EnableAutoExpandX = 1;
  flags.EnableAutoExpandY= 1;
  flags.EnableAutoShrinkY = 1;
  flags.EnableAutoShrinkX = 1;
  flags.EnableScroll_X = 1;
  flags.EnableScroll_Y = 1;
}

void uiMultiLine::operator<<(const std::string &str){ text += str; needRendering(true); }
void uiMultiLine::operator<<(const char *str){ text += std::string(str); needRendering(true); }
void uiMultiLine::operator<<(const int val){ text += std::to_string(val); needRendering(true); }
void uiMultiLine::operator<<(const double val){ text += std::to_string(val); needRendering(true); }

void uiMultiLine::render() {
  auto wnd = Engine::getDrawingWindow();
	if(!flags.Active){
		needRendering(false);
		std::cout << "Not visivle or not active widget\n";
		return;
	}
	if(flags.needRendering){
    const auto cl_tmp = wnd->getClippingRect();
    const auto s = wnd->AddString2D(text, pos-scrollPos, 1, uiRect(pos, size));
    if(s != outerSize){outerSize = s; impl_needCalcAlinment_parent(); }
    wnd->setClippingRect(cl_tmp);
		needRendering(false);
	}
  renderScrollbars();
}

bool uiMultiLine::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1,[ [maybe_unused]] int num_1, [[maybe_unused]] int num_2, [[maybe_unused]] const char **strings){
  CallbackScrollbars(flag, vec2_1, num_1, num_2, strings);
	return true;
}


std::vector<std::string> strsplit(const std::string &c, const std::string sep){
  const auto separator_length = sep.length(); // 区切り文字の長さ
  auto list = std::vector<std::string>();
  if (separator_length == 0) { list.push_back(c);
  } else {
    auto offset = std::string::size_type(0);
    while (1) {
      auto pos = c.find(sep, offset);
      if (pos == std::string::npos) {
        list.push_back(c.substr(offset));
        break;
      }
      list.push_back(c.substr(offset, pos - offset));
      offset = pos + separator_length;
    }
  }
  return list;
}

uiTable::uiTable(const std::string &text_){ setTextALl(text_); flags.EnableAutoExpandX = flags.EnableAutoShrinkX = 1;}

uiTable::uiTable(const std::vector<std::string> &text_, const int cols_){
  assert(cols_ > 0);
  assert(text_.size() > 0);
  cols = cols_;
  columns = text_;
  flags.EnableAutoExpandX = flags.EnableAutoShrinkX = 1;
}

void uiTable::setTextALl(const std::string &text_){
  if(text_.size() == 0){ uiLOGE << "uiTable str is empty!"; columns = {"<EMPTY STRING>"}; cols = 1; return;}
  const auto lines = strsplit(text_,"\n");
  if(lines.size() == 0){uiLOGE << "uiTable str is empty!"; columns = {"<EMPTY STRING>"}; cols = 1; return;}
  cols = 1;
  columns = {};
  bool tmp=true;
  for(const std::string &l : lines){
    uiLOGD << l;
    const auto td = strsplit(l, "\t");
    if(tmp && td.size() > 0){ cols = td.size(); tmp = false; }
    for(const std::string &t : td) columns.push_back(t);
  }
}

void uiTable::setCols(const int n){
  assert(n>0);
  cols = n;
  if(columns.size() < n){ columns.resize(n); }
}

void uiTable::setRows(const int n){
  assert(n>0);
  columns.resize(cols*n);
}

const std::string& uiTable::operator()(int x, int y) const{
  assert(x >= 0 && y >= 0);
  assert(x+y*cols < columns.size());
  return columns[y*cols + x];
}
std::string& uiTable::operator()(int x, int y){
  assert(x >= 0 && y >= 0);
  assert(x+y*cols < columns.size());
  return columns[y*cols + x];
}

void uiTable::render(){
  assert(cols > 0);
  assert(columns.size() >= cols);
  if(!flags.needRendering) return;
  // size[1] = std::max<int>(size[1], cols * 100);
  const int rows = columns.size() / cols;
  const int td_width = size[0] / cols;
  auto wnd = Engine::getDrawingWindow();
  const auto style = Engine::getStyle();
  int curHeight = pos[1];
  wnd->AddLine2D(pos,{pos[0]+size[0],pos[1]}, style->col_WidgetLineDisabled, 1.0);
  for(int y=0; y<rows; y++){
    int dh_max = 0;
    for(int x=0; x<cols; x++){
      const int idx = y*cols + x;
      if(columns.size() <= idx) break;
      const std::string &t = columns[idx];
      const Vector2d strPos = Vector2d{ pos[0] + td_width*x, curHeight } + style->WidgetPadding / 2;
      const auto ws = wnd->AddString2D(t, strPos, 1.0, col, td_width);
      dh_max = std::max<int>(dh_max, ws[1]+style->WidgetPadding[1]);
    }
    curHeight += dh_max;
    wnd->AddLine2D({pos[0], curHeight}, {pos[0]+size[0], curHeight}, style->col_WidgetLineDisabled, 1.0);
  }
  for(int x=0; x<=cols; x++) {
    const int X = pos[0] + td_width*x;
    wnd->AddLine2D({X, pos[1]}, {X, pos[1]+size[1]}, style->col_WidgetLineDisabled, 1.0);
  }

  const Vector2d new_outer_size = {size[1], curHeight - pos[1]};
  if(new_outer_size != outerSize){ impl_needCalcAlinment_parent(); outerSize = new_outer_size; }
  needRendering(false);  
}

bool uiTable::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) {return false;}

template <typename T>
uiVector3<T>::uiVector3(std::string text_, _Vec<T, 3> *value_, Vector2 range_, const double delta_){
    text = text_;
    value = value_;
    range = range_;
    delta = delta_;
	last_value = *value_;
	flags.EnableAutoExpandX = true;
	flags.EnableAutoShrinkX = true;
}
template <typename T>
void uiVector3<T>::render() {
	if(!flags.Active){
		needRendering(false);
		std::cout << "Not visivle or not active widget\n";
		return;
	}

  auto wnd = Engine::getDrawingWindow();
	const auto style = Engine::getStyle();
	if((*value != last_value) || (flags.needRendering)){
        const auto ss =wnd->AddString2D(text, pos + Vector2d{0,style->WidgetPadding[0]}, 1);
        const Vector2d pos2 {pos[0] + ss[0] + style->WidgetMargin[0], pos[1]}; 
        const Vector2d each_size { (size[0] - pos2[0] + pos[0]) / 3-1,  size[1]};
        _num_display_pos = pos2;
        if(each_size[0] < 0) return;
        Vector2d new_outer_size = ss;
        for(int i=0; i<3; i++){
            const Vector2d pos3 = pos2 + Vector2d{ each_size[0] * i, 0 };
            const auto col = (i == hovering_idx) ? style->col_WidgetBg : style->col_WidgetBgHover;
            if(i == hovering_idx){}
            wnd->AddRectPosSize(pos3, each_size, col);
            wnd->AddRectPosSize(pos3, each_size, style->col_WidgetLine, 1);
            char tt[20];
            if(range[1] - range[0] > 100){
              sprintf (tt, "%d", (int)(*value)[i]);
            }else{
              sprintf (tt, "%2.f", (float)(*value)[i]);
            }
            const auto s =wnd->AddString2D(std::string(tt), pos3, 1) + style->WidgetPadding;
            new_outer_size[0] = new_outer_size[0] + s[0];
            new_outer_size[1] = std::max(s[1], new_outer_size[1]) ;
            last_value = *value;
        }
        new_outer_size += style->WidgetPadding*2;
        new_outer_size[0] = std::max<int>(new_outer_size[0], 20);
        new_outer_size[1] = std::max<int>(new_outer_size[1], 10);
        if(new_outer_size != outerSize){ impl_needCalcAlinment_parent(); outerSize = new_outer_size; }
		needRendering(false);
	}
}
template <typename T>
bool uiVector3<T>::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1,[ [maybe_unused]] int num_1, [[maybe_unused]] int num_2, [[maybe_unused]] const char **strings){
  const int each_size = (size[0] - _num_display_pos[0] + pos[0]) / 3;
  int tmp_idx = (vec2_1[0]- _num_display_pos[0]) / each_size;
  if(tmp_idx > 2 || vec2_1[1] < pos[1] || pos[1] + size[1] < vec2_1[1] || vec2_1[0] < _num_display_pos[0]) tmp_idx = -1;
	if ( flag == uiCallbackFlags::LMouseDown || flag ==uiCallbackFlags::MouseScroll){ selecting_idx = tmp_idx; 
  }else if (flag ==uiCallbackFlags::MouseMove){ hovering_idx = tmp_idx; }

  if(selecting_idx < 0){ return false; }
  if(Engine::getFocusedWidget() != this) return false;  

 if(flag ==uiCallbackFlags::MouseScroll){
		(*value)[selecting_idx] += num_2 > 0 ? 1 : -1;
    return true;
	}else if(flag == uiCallbackFlags::LMouseDown){
    start_mouse_pos_x = vec2_1[0];
    start_mouse_val = *value;
  }else if(flag ==uiCallbackFlags::MouseMove && (num_2 & 0b01)){
    (*value)[selecting_idx] = start_mouse_val[selecting_idx] + delta * (vec2_1[0] - start_mouse_pos_x);
	}
	
  (*value)[0] = std::clamp<float>((*value)[0], range[0], range[1]);
	(*value)[1] = std::clamp<float>((*value)[1], range[0], range[1]);
	(*value)[2] = std::clamp<float>((*value)[2], range[0], range[1]);
	return true;
}

// the explicit instanciation
template uiVector3<double>::uiVector3(std::string, _Vec<double, 3> *, Vector2, const double);
template uiVector3<float>::uiVector3(std::string, _Vec<float, 3> *, Vector2, const double);
template uiVector3<int>::uiVector3(std::string, _Vec<int, 3> *, Vector2, const double);
template uiVector3<unsigned int>::uiVector3(std::string, _Vec<unsigned int, 3> *, Vector2, const double);


uiCol::uiCol(const std::string &text_, Vector3b *value_){
  text = text_;
  value = value_;
	last_value = *value_;
	flags.EnableAutoExpandX = true;
	flags.EnableAutoShrinkX = true;
}
void uiCol::render() {
	if(!flags.Active){needRendering(false); return;	}
  auto wnd = Engine::getDrawingWindow();
	const auto style = Engine::getStyle();
	if(*value == last_value && (!flags.needRendering)) return;

  const auto ss =wnd->AddString2D(text, pos + Vector2d{0,style->WidgetPadding[0]}, 1);
  const Vector2d pos2_ {pos[0] + ss[0] + style->WidgetMargin[0], pos[1]}; 
  wnd->AddRectPosSize(pos2_, {size[1], size[1]}, *value);
  wnd->AddRectPosSize(pos2_, {size[1], size[1]}, style->col_WidgetLine, 1);
  const auto pos2 = pos2_ + Vector2d(size[1]+1, 0);
  const Vector2d each_size { (size[0] - pos2[0] + pos[0]) / 3-1,  size[1]};
  _num_display_pos = pos2;
  if(each_size[0] < 0) return;
  Vector2d new_outer_size = ss;
  for(int i=0; i<3; i++){
    const Vector2d pos3 = pos2 + Vector2d{ each_size[0] * i, 0 };
    const auto col = (i == selecting_idx) ? style->col_WidgetBg : style->col_WidgetBgHover;
    wnd->AddRectPosSize(pos3, each_size, col);
    wnd->AddRectPosSize(pos3, each_size, style->col_WidgetLine, 1);
    const auto s =wnd->AddString2D(std::to_string((*value)[i]), pos3, 1) + style->WidgetPadding;
    new_outer_size[0] = new_outer_size[0] + s[0];
    new_outer_size[1] = std::max(s[1], new_outer_size[1]) ;
    last_value = *value;
  }
  new_outer_size += style->WidgetPadding*2;
  new_outer_size[0] = std::max<int>(new_outer_size[0], 20);
  new_outer_size[1] = std::max<int>(new_outer_size[1], 10);
  if(new_outer_size != outerSize){ impl_needCalcAlinment_parent(); outerSize = new_outer_size; }
	needRendering(false);
}

bool uiCol::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1,[ [maybe_unused]] int num_1, [[maybe_unused]] int num_2, [[maybe_unused]] const char **strings){
  constexpr int delta = 1;
  Vector3 tmp_v{(double)(*value)[0], (double)(*value)[1], (double)(*value)[2]};
  const int each_size = (size[0] - _num_display_pos[0] + pos[0]) / 3;
  int tmp_idx = (vec2_1[0]- _num_display_pos[0]) / each_size;
  if(tmp_idx > 2 || vec2_1[1] < pos[1] || pos[1] + size[1] < vec2_1[1] || vec2_1[0] < _num_display_pos[0]) tmp_idx = -1;
	if ( flag == uiCallbackFlags::LMouseDown || flag ==uiCallbackFlags::MouseScroll){ selecting_idx = tmp_idx; }
  if(selecting_idx < 0){ return false; }
  if(Engine::getFocusedWidget() != this) return false;  

 if(flag ==uiCallbackFlags::MouseScroll){
		std::cout << "MOUSESCROLL" << vec2_1[0] << vec2_1[1] << std::endl;
		tmp_v[selecting_idx] += num_2 > 0 ? delta : -delta;
    return true;
	}else if(flag == uiCallbackFlags::LMouseDown){
    start_mouse_pos_x = vec2_1[0];
    start_mouse_val = tmp_v;
  }else if(flag ==uiCallbackFlags::MouseMove && (num_2 & 0b01)){
    tmp_v[selecting_idx] = start_mouse_val[selecting_idx] + delta * (vec2_1[0] - start_mouse_pos_x);
	}
	
  (*value)[0] = std::clamp<int>(tmp_v[0], 0, 255);
	(*value)[1] = std::clamp<int>(tmp_v[1], 0, 255);
	(*value)[2] = std::clamp<int>(tmp_v[2], 0, 255);
	return true;
}



uiFrame::uiFrame(std::string text_){ 
  title = text_;
	needRendering(true);
	setAlignType(uiWidgetAlignTypes::VertialListl);
	flags.EnableAutoExpandX = 1;
	flags.EnableAutoExpandY = 1;
	flags.EnableAutoShrinkX = 1;
	flags.EnableAutoShrinkY = 1;
	flags.EnableUserResize  = 1;
    // flags.EnableScroll_X = 1;
    flags.EnableScroll_Y = 1;
    setSize({200,200});
}

void uiFrame::render() {
    if(!flags.Active){
        needRendering(false);
        std::cout << "Not visivle or not active widget\n";
        return;
    }
    auto wnd = Engine::getDrawingWindow();
	const auto style = Engine::getStyle();
	
	// if(lastScrollPos != scrollPos){ needRendering(true); }
     const uint16_t title_bar = flags.EnableTitlebar ? style->TitlebarHeight: 0;
	if(flags.needRendering > 0){
		// if(flags.CollapsingTitlebar && flags.EnableTitlebar) size[1] = style->TitlebarHeight; 
		// else CollapseTitlebar(false);
        if(flags.EnableTitlebar){
            if(last_collapsing !=flags.CollapsingTitlebar){
                if(flags.CollapsingTitlebar){ last_not_collapse_size = size[1];}
                else  { size[1] = last_not_collapse_size; }
                last_collapsing = flags.CollapsingTitlebar;
            } 
            if(flags.CollapsingTitlebar){ size[1] = title_bar; }
        }
        wnd->setClippingRect(std::move(vkUI::uiRect(pos+Vector2d{0, title_bar}, size - Vector2d{0, title_bar})));
		wnd->AddRectPosSize(pos, size, style->col_WidgetBg);
		wnd->AddRectPosSize(pos, size, style->col_WidgetLine, 2);

    // draw resozer
    constexpr int resizer_size = 10;
    wnd->AddTriangle2D(
      pos+size - Vector2d{resizer_size, 0},
      pos+size - Vector2d{0, 0},
      pos+size - Vector2d{0, resizer_size},
      Vector3b{100,100,200}
    );

		if(title != "" ) flags.EnableTitlebar = 1;
		if(flags.EnableTitlebar){
			wnd->AddRectPosSize(pos, {size[0], title_bar}, style->col_WidgetBgHover);
			wnd->AddRectPosSize(pos, {size[0], title_bar}, style->col_PopupBg, 1);
			if(flags.CollapsingTitlebar) wnd->AddArrowRight2D(pos + 2, title_bar-5, style->col_Text);
			else wnd->AddArrowDown2D(pos + title_bar/3, title_bar*0.8, style->col_Text);
			const auto s = wnd->AddString2D(title, pos + Vector2d{title_bar + 3, 2}, 1, style->col_Text);
            wnd->AddCrossButton(pos + Vector2d{size[0] - title_bar+1, 1}, title_bar-2, style->col_WidgetBg, style->col_WidgetLine, style->col_Text);
            size[0] = std::max(s[0] + title_bar, size[0]);
		}
	}

	/* DrawList[0].setClipRect(pos_ + Vector2d(0, title_bar), size_ - Vector2d(0, title_bar)); */
	if(!flags.CollapsingTitlebar && flags.EnableTitlebar){
		for(uint16_t i=0; i<widgets.size(); i++) widgets[i]->render();
		renderScrollbars(); 
	}else{
		// innerSize = size;
        outerSize = size;
	}
	needRendering(false);
}

bool uiFrame::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1,[ [maybe_unused]] int num_1, [[maybe_unused]] int num_2, [[maybe_unused]] const char **strings) {
    if(!flags.Active) return false;
    CallbackScrollbars(flag, vec2_1, num_1, num_2, strings);
	CallbackTitlebar(flag, vec2_1, num_1, num_2, strings);
	CallbackResizer(flag, vec2_1, num_1, num_2, strings);
  updateScrollPos();
  return true;
}



uiCollapse::uiCollapse(std::string text_){ 
  title = text_;
	needRendering(true);
	setAlignType(uiWidgetAlignTypes::VertialListl);
	flags.EnableAutoExpandX = 1;
  flags.EnableTitlebar    = 1;
	flags.EnableAutoExpandY = 0;
	flags.EnableAutoShrinkX = 1;
	flags.EnableAutoShrinkY = 0;
	flags.EnableUserResize  = 0;
    // flags.EnableScroll_X = 1;
    flags.EnableScroll_Y = 0;
}

void uiCollapse::render() {
   if(!flags.Active){
    needRendering(false);
    std::cout << "Not visivle or not active widget\n";
    return;
  }
  auto wnd = Engine::getDrawingWindow();
	const auto style = Engine::getStyle();
	
    Vector2d whole_size{0, 0};
	// if(lastScrollPos != scrollPos){ needRendering(true); }
     const uint16_t title_bar = style->TitlebarHeight;
	if(!flags.needRendering) return;

    // wnd->setClippingRect(std::move(vkUI::uiRect(pos+Vector2d{0, title_bar}, size - Vector2d{0, title_bar})));
    // if(title != "" ) flags.EnableTitlebar = 1;
    wnd->AddRectPosSize(pos, {size[0], title_bar}, {50,50,90});
    if(flags.CollapsingTitlebar){ wnd->AddArrowRight2D(pos + 2, title_bar-5, style->col_Text);}
    else{ wnd->AddArrowDown2D(pos + title_bar/3.0f, title_bar*0.8, style->col_Text); }
    const auto s = wnd->AddString2D(title, pos + Vector2d{title_bar + 3, 2}, 1, style->col_Text);
    whole_size = Vector2d{ s[0] + title_bar, s[1] } + style->WidgetMargin;
    if(!flags.CollapsingTitlebar){
        for(uint16_t i=0; i<widgets.size(); i++) {
            widgets[i]->render();
            const auto sw = widgets[i]->getSize();
            whole_size = {
                std::max<int>(whole_size[0], sw[0] + style->WidgetMargin[0]),
                whole_size[1] + sw[1] + style->WidgetMargin[1]*2
            };
        }
    }
    if(whole_size != outerSize){
      outerSize = whole_size;
      impl_needCalcAlinment_parent();
    }
	needRendering(false);
}

bool uiCollapse::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings) {
    if(!flags.Active) return false;
    if(flag != uiCallbackFlags::LMouseDown) return false;
    /* if(!flags.EnableTitlebar ){ return false; } */
    const int th = Engine::getStyle()->TitlebarHeight;
    if(!uiRect(pos, {size[0] - th - 6, th}).isContains(vec2_1)) return false;
    
    CollapseTitlebar(!flags.CollapsingTitlebar);
    impl_needCalcAlignment_all(); //TODO:Frame in Frameの設計がなされている時はParent->parentにする必要があるような....
    return false;
}


// -----------------------------------------------------
//   [SECTION]   General Funcs
// -----------------------------------------------------

void uiWidget::renderScrollbars(){
    const auto style = Engine::getStyle();
    const auto wnd = Engine::getDrawingWindow();
    // if((lastInnerSize != innerSize) || (scrollPos != lastScrollPos)){ needRendering(true); if(widgets.size() > 0) needApplyAlignment();}
    const auto innerSize =  getOuterSize() - style->WidgetPadding - Vector2d(0, - style->TitlebarHeight); // TODO: Widgetごとに異なる可能性があるのでgetInnerSize()にする？それともスクロールバーRendering実装をWidget個別にする＿

    Vector2d pos_scrollX = pos + Vector2d{ 0, size[1] - style->ScrollbarWidth };
    Vector2d pos_scrollY = pos + Vector2d{ size[0] - style->ScrollbarWidth, 0};

    if(innerSize[1] > size[1] && flags.needRendering > 0){
        wnd->AddRectPosSize(pos_scrollY, Vector2d(style->ScrollbarWidth, size[1] - style->ScrollbarWidth), style->col_wndScrollBg);
        wnd->AddRectPosSize(Vector2d(pos_scrollY[0]+1, pos[1] + scrollPos[1]*size[1]/innerSize[1]), 
                                 Vector2d(style->ScrollbarWidth-1, (size[1] - style->ScrollbarWidth)*size[1]/innerSize[1]), style->col_wndScroll);
    }

    if(innerSize[0] > size[0] && flags.needRendering > 0){
        wnd->AddRectPosSize(pos_scrollX, Vector2d(size[0] - style->ScrollbarWidth,  style->ScrollbarWidth), style->col_wndScrollBg);
        wnd->AddRectPosSize(Vector2d(pos[0] + scrollPos[0]*size[0]/innerSize[0], pos_scrollX[1]+1), 
                                 Vector2d(size[0]*(size[0] - style->ScrollbarWidth)/innerSize[0], style->ScrollbarWidth-1), style->col_wndScroll);
    }
    // lastInnerSize = innerSize;
    // lastScrollPos = scrollPos;
}

void uiWidget::CallbackTitlebar(uiCallbackFlags flag, Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings){
  static bool is_moving = false;
  static Vector2d moving_mouse_offset{0,0};
  static Vector2d move_start_pos = pos;
  
  const int th = Engine::getStyle()->TitlebarHeight;
  const bool touching_closebtn= uiRect(pos[0]+size[0]-th, pos[1], th, th).isContains(vec2_1);
  const bool touching_titlebar = uiRect(pos, {size[0] - th - 6, th}).isContains(vec2_1);

  if(flag == uiCallbackFlags::MouseMove && is_moving && (num_2 & 0x01) ){
    setPos(vec2_1 - moving_mouse_offset);
    impl_needCalcAlinment_parent(); 
  }else if(flag == uiCallbackFlags::LMouseDown && touching_titlebar && flags.Active){
    const auto parent = getParentWidget();
    if(parent == this || parent->align_type != uiWidgetAlignTypes::Absolute){ return; }
    moving_mouse_offset = vec2_1 - pos;
    move_start_pos = pos; 
    is_moving = true;
    impl_needCalcAlinment_parent();
  }else if(flag == uiCallbackFlags::LMouseUP && move_start_pos == pos) {
    is_moving = false;
    if(!flags.EnableTitlebar ){ return; }
    if(touching_closebtn) { setActive(false); }
    else if(touching_titlebar) { 
      if(parent->align_type == uiWidgetAlignTypes::Absolute && !parent->isRenderedTop(this)) parent->toRenderTop(this);
      else CollapseTitlebar(!flags.CollapsingTitlebar); 
    }
    impl_needCalcAlinment_parent(); //TODO:Frame in Frameの設計がなされている時はParent->parentにする必要があるような....
  }else{
    is_moving = false;
  }
}

void uiWidget::updateScrollPos(){
  const auto style = Engine::getStyle();
  if(!flags.EnableTitlebar) return;
  const auto innerSize =  getOuterSize() - style->WidgetPadding - Vector2d(0, - style->TitlebarHeight); // TODO: Widgetごとに異なる可能性があるのでgetInnerSize()にする？それともスクロールバーRendering実装をWidget個別にする＿
  const auto expand_size = innerSize - size;
  if(flags.EnableScroll_Y && expand_size[0] > 0) scrollPos[0] = std::clamp<int>(scrollPos[0], 0, expand_size[0]);
  if(flags.EnableScroll_Y && expand_size[1] > 0) scrollPos[1] = std::clamp<int>(scrollPos[1], 0, expand_size[1]);
}


bool uiWidget::CallbackResizer(uiCallbackFlags flag, Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings){
    if(!(flag == uiCallbackFlags::LMouseDown || flag == uiCallbackFlags::LMouseUP
      || flag == uiCallbackFlags::MouseMove  || flag == uiCallbackFlags::OffHover)) return false;
    const auto wnd = Engine::getDrawingWindow(); 

    constexpr int ui_inner_gap_ = 3;
    const int th = Engine::getStyle()->TitlebarHeight + 4;
    constexpr int resizer_size = 10;
    uiRect resizeRect_ld(pos[0] + size[0] - resizer_size, pos[1] + size[1] - resizer_size, resizer_size, resizer_size); 
    // uiRect resizeRect_top  (pos[0]                         , pos[1],                          size[0], ui_inner_gap_); 
    uiRect resizeRect_btm  (pos[0]                         , pos[1] + size[1] - ui_inner_gap_, size[0], ui_inner_gap_); 
    uiRect resizeRect_left (pos[0]                         , pos[1] + th,                          ui_inner_gap_, size[1] - th); 
    uiRect resizeRect_right(pos[0] + size[0] - ui_inner_gap_, pos[1] + th,                          ui_inner_gap_, size[1] - th); 

    if(flags.EnableUserResize && !flags.CollapsingTitlebar){
        bool isMouseOn = ((flag == uiCallbackFlags::MouseMove) && ((num_2 & 0x01) != 0)) || flag == uiCallbackFlags::LMouseDown;
        // リサイズ開始かどうかを判断
        if(flag == uiCallbackFlags::LMouseDown){
            if(resizeRect_ld.isContains(vec2_1[0], vec2_1[1])){
                isSelectingResizer = true;
                wnd->setCrossHairCursor();
            }else if(resizeRect_btm.isContains(vec2_1[0], vec2_1[1])){
                isSelectingEdge_bottom = true;
                wnd->setVResizeCursor();
            }else if(resizeRect_left.isContains(vec2_1[0], vec2_1[1])){
                isSelectingEdge_left = true;
                wnd->setHResizeCursor();
            }else if(resizeRect_right.isContains(vec2_1[0], vec2_1[1])){
                isSelectingEdge_right = true;
                wnd->setHResizeCursor();
            }
        }

        if(isSelectingResizer){
            if(isMouseOn) {
                setWidth (vec2_1[0] - pos[0]);
                setHeight(vec2_1[1] - pos[1]);
                needRendering(true);
                impl_needCalcAlinment_parent();
                return true;
            } else if(flag == uiCallbackFlags::LMouseUP) {
                isSelectingResizer = false;
                parent->needRendering(true);
                wnd->setDefaultCursor();
                return true;
            }
        }
        if(isSelectingEdge_left){
            if(isMouseOn) {
            //    (flag == uiCallbackFlags:::LMouseDown && ))
                setWidth(size[0] - vec2_1[0] + pos[0]);
                setPosX(vec2_1[0]);
                impl_needCalcAlinment_parent();
                parent->needRendering(true);
                return true;
            } else if(flag == uiCallbackFlags::LMouseUP) {
                isSelectingEdge_left = false;
                parent->needRendering(true);
                wnd->setDefaultCursor();
                return true;

            }
        }
        if(isSelectingEdge_right){
            if(isMouseOn) {
            //    (flag == uiCallbackFlags:::LMouseDown && ))
                setWidth(vec2_1[0] - pos[0]);
                impl_needCalcAlinment_parent();
                parent->needRendering(true);
                return true;
            } else if(flag == uiCallbackFlags::LMouseUP) {
                isSelectingEdge_right = false;
                parent->needRendering(true);
                wnd->setDefaultCursor();
                return true;
            }
        }
        if(isSelectingEdge_bottom){
            if(isMouseOn) {
                setHeight(vec2_1[1] - pos[1]);
                impl_needCalcAlinment_parent();
                parent->needRendering(true);
                return true;
            } else if(flag == uiCallbackFlags::LMouseUP) {
                isSelectingEdge_bottom = false;
                parent->needRendering(true);
                wnd->setDefaultCursor();
                return true;
            }
        // }else if(isSelectingEdge_top){
        //     if(isMouseOn) {
        //         setPosY(vec2_1[1]);
        //         setHeight(vec2_1[1] - pos[1]);
        //         parent->needRendering(true);
        //         return true;
        //     } else if(flag == uiCallbackFlags::LMouseUP) {
        //         isSelectingEdge_top = false;
        //         setHeight(vec2_1[1] - pos[1]);
        //         setPosY(vec2_1[1]);
        //         parent->needRendering(true);
        //         glfwSetCursor(window, NULL);
        //         return true;
        //     }
        }
    }
    // for(int i=0; i<widgets.size(); i++){
    //     if(widgets[i]->CallbackResizer(flag, vec2_1, num_1, num_2, strings)) return true;
    // }
    return false;
}

bool uiWidget::CallbackScrollbars(uiCallbackFlags flag, Vector2d vec2_1,[ [maybe_unused]] int num_1, [[maybe_unused]] int num_2, [[maybe_unused]] const char **strings){
    auto style = Engine::getStyle();
    bool isContains;
    const auto innerSize =  getOuterSize() - style->WidgetPadding - Vector2d(0, - style->TitlebarHeight); // TODO: Widgetごとに異なる可能性があるのでgetInnerSize()にする？それともスクロールバーRendering実装をWidget個別にする＿
    // Vertical
    if(innerSize[1] >= size[1]){
        isContains = (pos[0] + size[0] - style->ScrollbarWidth <= vec2_1[0]) && (vec2_1[0] <= pos[0] + size[0]) && 
                     (pos[1] <= vec2_1[1]) && (vec2_1[1] < pos[1] + size[1]-30);
        if(flag ==uiCallbackFlags::LMouseUP) isSelectingScrollY = false; 
        if(((flag ==uiCallbackFlags::LMouseDown) && isContains) || isSelectingScrollY){ 
            int tmp_sy =  (float(vec2_1[1]) - float(pos[1]) - float((size[1] - style->ScrollbarWidth)*size[1])/(innerSize[1]*2.0))*innerSize[1]/size[1];
            isSelectingScrollY = true;
            scrollPos[1] = std::clamp<int>(tmp_sy, 0, innerSize[1] - size[1]);
            needRendering(true);
            impl_needCalcAlinment_parent();
            return true;
        }
        if(flag ==uiCallbackFlags::MouseScroll && num_2 != 0){
            scrollPos[1] = std::clamp<int>(scrollPos[1] - num_2*15, 0, innerSize[1] - size[1]);
            impl_needCalcAlinment_parent();
            needRendering(true);
            return true;
        }
    }else{
        if(flag ==uiCallbackFlags::MouseScroll && num_2 != 0){
            if(parent != this) return parent->CallbackScrollbars(flag, vec2_1, num_1, num_2, strings);
        }
    }


    if(innerSize[0] >= size[0]){
        //Horizontal
        isContains = (pos[0] <= vec2_1[0]) && (vec2_1[0] <= pos[0] + size[0]) && (pos[1] + size[1] - style->ScrollbarWidth <= vec2_1[1]) && (vec2_1[1] < pos[1] + size[1]);
        if(flag ==uiCallbackFlags::LMouseUP) isSelectingScrollX = false;
        if(((flag ==uiCallbackFlags::LMouseDown) && isContains) || isSelectingScrollX){
            int tmp_sy =  (float(vec2_1[0]) - float(pos[0]) - float(size[0]*(size[0] - style->ScrollbarWidth))/(innerSize[0]*2.0))* innerSize[0]/size[0] ;
            isSelectingScrollX = true;
            scrollPos[0]  = std::clamp<int>(tmp_sy, 0, innerSize[0] - size[0]);
            needRendering(true);
            return true;
        }
    }else{
        if(flag ==uiCallbackFlags::MouseScroll && num_1 != 0){
            if(parent != this) return parent->CallbackScrollbars(flag, vec2_1, num_1, num_2, strings);
        }
    }
    return false;
}


// -----------------------------------------------------
//   [SECTION]   Calc alignment
// -----------------------------------------------------

void uiWidget::calcAlignVertical(){
  if(widgets.size() == 0) return; 
  if(flags.EnableTitlebar && flags.CollapsingTitlebar) return;
  const auto style = Engine::getStyle();
	const uint16_t titlebar_y  = flags.EnableTitlebar ? style->TitlebarHeight : 0;

  uint16_t desiredHeight = 0;
  uint16_t desiredWidth = 0;
  uint16_t numExHeightWidgets = 0;
  uint16_t numShHeightWidgets = 0;

  bool EnScrollX = false;
  bool EnScrollY = false; // thisウィジェットのスクロールバーを表示するか（右と下にスペースを作るか
  for(auto w:widgets){
      if(w->getEnableAutoExpandY()) numExHeightWidgets ++;
      if(w->getEnableAutoShrinkY()) numShHeightWidgets ++;
      EnScrollX |= !(w->getEnableAutoExpandX());
      EnScrollY |= !(w->getEnableAutoExpandY());
  }

  for(auto w : widgets){
      const auto tmpCurS = w->getOuterSize(); //getSize();
      //WIDTH SETTINGS:
      const int this_frame_inner_size = std::max<int>(0, size[0] - style->WidgetMargin[0]*2 - (EnScrollX ? (style->ScrollbarWidth) : 0));
      const bool x_overflow = this_frame_inner_size >= tmpCurS[0];
      if(x_overflow){
           w->setWidth(w->getEnableAutoExpandX() ? this_frame_inner_size : tmpCurS[0] ); 
      }else{
           w->setWidth(w->getEnableAutoShrinkX() ? this_frame_inner_size : tmpCurS[0] ); 
      }
      desiredHeight += tmpCurS[1] + style->WidgetMargin[1];
  }

  // 高さの調整（各Widgetごとに拡大縮小
  const auto height_max = size[1]- (EnScrollY ? (style->ScrollbarWidth) : 0) - titlebar_y;
  if(desiredHeight < height_max){
    if(numExHeightWidgets > 0){
     const int expand_each = (height_max - desiredHeight) / numExHeightWidgets; //拡大できるサイズ
     assert(expand_each >= 0);
     for(auto w : widgets){
      if(w->getEnableAutoExpandY()){ w->setHeight(std::min<int>(w->getOuterSize()[1] + expand_each, height_max)); }
      else                         { w->setHeight(std::min<int>(w->getOuterSize()[1], height_max)); }
     }
    }else{
      for(auto w : widgets) {  w->setHeight(w->getOuterSize()[1]);}
    }
  }else{
    if(numShHeightWidgets > 0){
      const int shrink_each =  (desiredHeight - height_max) / numShHeightWidgets;
      assert(shrink_each > 0);
      for(auto w: widgets){
          if(w->getEnableAutoShrinkY()){ w->setHeight(std::max<int>(w->getOuterSize()[1] - shrink_each, style->WidgetMinSize[1])); }
          else                         { w->setHeight(w->getOuterSize()[1]); }
      }
    }else{
      for(auto w:widgets) w->setHeight(w->getOuterSize()[1]);
    }
  }

  // それぞれのサイズの確認と表示位置の設定
  desiredWidth  = 0; desiredHeight  = 0;
  for(auto w:widgets){
      const auto tmpCurS = w->getSize();
      w->setPos(Vector2d(pos[0] + style->WidgetMargin[0] - scrollPos[0], pos[1] + titlebar_y + desiredHeight - scrollPos[1]));
      desiredWidth = std::max<int>(desiredWidth, tmpCurS[0]);
      desiredHeight += tmpCurS[1] + style->WidgetMargin[1];
  }

  // setSize(desiredWholeSize, -1)
  // innerSize[0] = desiredWidth;
  // innerSize[1] = desiredHeight + titlebar_y;
  outerSize = style->WidgetPadding + Vector2d(desiredWidth, desiredHeight + titlebar_y);
 
  needRendering(true);
  flags.needApplyAlignment = 0;
  flags.needCalcInnerSize = 0;
}


void uiWidget::calcAlignHorizontal(){
    /*
  if(widgets.size() == 0) { flags.needApplyAlignment = false; flags.needCalcInnerSize = false; return; }
  const auto style = Engine::getStyle();
    
	uint16_t titlebar_y = 0;
	if(flags.EnableTitlebar) titlebar_y  = style->FontSize + style->WidgetInnerSpace_y*2 + 4;
    
    Vector2d outerSpace;
    outerSpace[0] = spacing ? (style->WidgetOuterSpace_x) : 0; 
    outerSpace[1] = spacing ? (style->WidgetOuterSpace_y) : 0; 

    uint16_t desiredWidth = 0;
    uint16_t desiredHeight = 0;
    uint16_t numExWidthWidgets = 0;
    uint16_t numShWidthWidgets = 0;
    Vector2d tmpMinS, tmpMaxS, tmpCurS;

    bool EnScrollX = false;
    bool EnScrollY = false; // thisウィジェットのスクロールバーを表示するか（右と下にスペースを作るか
    for(uint16_t i = 0; i<widgets.size(); i++){
        if(widgets[i]->getEnableAutoExpandX()) numExWidthWidgets ++;
        if(widgets[i]->getEnableAutoShrinkX()) numShWidthWidgets ++;

        EnScrollX |= !(widgets[i]->getEnableAutoExpandX());
        EnScrollY |= !(widgets[i]->getEnableAutoExpandY());
    }
 
    for(uint16_t i =0; i< widgets.size(); i++){
        tmpCurS = widgets[i]->getInnerSize(); //getSize();
        std::cout << "render size = (" << tmpCurS[0] << ", " << tmpCurS[1] << ")\n";

        //HEIGHT SETTINGS:
        if(size[1] - outerSpace[1] -  (EnScrollY ? (style->ScrollbarWidth) : 0) >= tmpCurS[1] + outerSpace[1]*2){
            if(widgets[i]->getEnableAutoExpandY()){ widgets[i]->setHeight(size[1] - outerSpace[1] - (EnScrollY ? (style->ScrollbarWidth) : 0) - titlebar_y); }else{ widgets[i]->setWidth(tmpCurS[1]); }
        }else{
            if(widgets[i]->getEnableAutoShrinkY()){ widgets[i]->setHeight(size[1] - outerSpace[1] - (EnScrollY ? (style->ScrollbarWidth) : 0) - titlebar_y); }else{ widgets[i]->setWidth(tmpCurS[1]); }
        }

        //HEIGHT SETTINGS:
        desiredWidth += tmpCurS[0] + outerSpace[1]*2;
    }

    // 高さの調整（各Widgetごとに拡大縮小
    if(desiredWidth < size[0] -  (EnScrollX ? (style->ScrollbarWidth) : 0)){
        if(numExWidthWidgets > 0){
            int expand_each = (size[0] - desiredWidth - (EnScrollX ? (style->ScrollbarWidth) : 0)) / numExWidthWidgets; //拡大できるサイズ
            for(int i=0; i<widgets.size(); i++){
                if(widgets[i]->getEnableAutoExpandY()){ widgets[i]->setWidth(std::min<int>(widgets[i]->getInnerSize()[0] + expand_each, size[0] - (EnScrollX ? (style->ScrollbarWidth) : 0) )); }
                else                                  { widgets[i]->setWidth(std::min<int>(widgets[i]->getInnerSize()[0],               size[0] - (EnScrollX ? (style->ScrollbarWidth) : 0) )); }
            }
        }else{
            for(int i=0; i<widgets.size(); i++) widgets[i]->setHeight(widgets[i]->getInnerSize()[0]);
        }
    }else{
        if(numShWidthWidgets > 0){
            int shrink_each =  (desiredWidth + (EnScrollX ? (style->ScrollbarWidth) : 0) - size[0]) / numShWidthWidgets;
            for(int i=0; i<widgets.size(); i++){
                if(widgets[i]->getEnableAutoShrinkY()){ widgets[i]->setWidth(std::max<int>(int(widgets[i]->getInnerSize()[0]) - int(shrink_each), style->WidgetMinSize_y)); }
                else                                  { widgets[i]->setWidth(widgets[i]->getInnerSize()[0]); }
            }
        }else{
            for(int i=0; i<widgets.size(); i++) widgets[i]->setWidth(widgets[i]->getInnerSize()[0]);
        }
    }

    // それぞれのサイズの確認と表示位置の設定
    desiredWidth  = 0; desiredWidth  = 0;
    for(int i = 0; i<widgets.size(); i++){
        tmpCurS = widgets[i]->getSize();
        widgets[i]->setPos(Vector2d(pos[0] + outerSpace[0] - scrollPos[0] + desiredWidth, pos[1] + titlebar_y));
        std::cout << "position : " << widgets[i]->getPosX() << ", " << widgets[i]->getPosY() << std::endl;
        desiredWidth += tmpCurS[0] + outerSpace[1]*2;
        desiredHeight = std::max<int>(desiredHeight, tmpCurS[0]);
    }

    innerSize[1] = desiredHeight + titlebar_y;
    innerSize[0] = desiredWidth;
    needRendering(true);
    flags.needApplyAlignment = 0;
    flags.needCalcInnerSize = 0;
    */
}

}
