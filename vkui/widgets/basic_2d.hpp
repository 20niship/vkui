#pragma once
#include <functional>
#include <widget.hpp>
#include <engine.hpp>

namespace vkUI{


class uiLabel : public uiWidget{
private:
    std::string text;
    Vector3b col;
public:
	uiLabel() = delete;
	uiLabel(const uiLabel&) = delete;
	uiLabel& operator=(const uiLabel&) = delete;
    
  uiLabel(const std::string text_);
  uiLabel(const std::string text_, const Vector3b &col);
	void render() override;
	void setText(const std::string &str){ text = str; needRendering(true); }
	std::string getText() const { return text; }
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};


class uiButton : public uiWidget{
private:
	bool *value;
	bool last_value;
    std::string text;
public:
	uiButton() = delete;
	uiButton(const uiButton&) = delete;
	uiButton& operator=(const uiButton&) = delete;

    uiButton(std::string text_, bool *value_);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};

class uiButtonFunc : public uiWidget{
private:
  std::function<void(void)> func;
  std::string text;
public:
	uiButtonFunc() = delete;
	uiButtonFunc(const uiButton&) = delete;
	uiButtonFunc& operator=(const uiButton&) = delete;
  uiButtonFunc(const std::string text_, std::function<void(void)> func_);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};
class uiCheckbox : public uiWidget{
private:
	bool *value;
	bool last_value;
    std::string text;
public:
	uiCheckbox() = delete;
	uiCheckbox(const uiCheckbox&) = delete;
	uiCheckbox& operator=(const uiCheckbox&) = delete;
    uiCheckbox(std::string text_, bool *value_);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};

template <typename T>
class uiSliderH : public uiWidget{
private:
	T *value;
	T last_value;
	Vector2 range;
	std::string text;
	float delta{1.0f};
	// float last_value, min_value, max_value, delta_value;
public:
  uiSliderH(const std::string &text_, T *value_, const Vector2 &range_, const double delta_=1.0);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};


class uiMultiLine : public uiWidget{
private:
  std::string text;
public:
  uiMultiLine(const std::string text_ = "");
  void clear(){ text = ""; }
  void operator<<(const std::string &str);
  void operator<<(const char *str);
  void operator<<(const int var);
  void operator<<(const double var);  

  static const std::string endl(){ return "\n"; }
  static const std::string red(){ return "<#FF0000>"; }
  static const std::string white(){ return "<#FFFFFF>"; }
  static const std::string green(){ return "<#00FF00>"; }
  static const std::string blue(){ return "<#0000FF>"; }
  /* static const std::string col(const Vector3b &col){ return "<#F0F0F0>"; std::cout << "not implemented" << col << std::endl; } */

	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};

class uiTable : public uiWidget{
private:
  std::vector<std::string> columns;
  int cols;
  Vector3b col{255,255,255};
public:
	uiTable() = delete;
	uiTable(const uiTable&) = delete;
	uiTable& operator=(const uiTable&) = delete;
  uiTable(const std::string &text_); // \t for next column, \r for next row
  uiTable(const std::vector<std::string> &text_, const int cols); 
  void setTextALl(const std::string &); // \t for next column, \r for next row
  void setCols(const int n);
  void setRows(const int n);
  void setSize(const int x, const int y){ setCols(x); setRows(y);}
  int getCols()const { return cols; }
  int getRows()const { return columns.size() / cols; }
  const std::string& operator()(int x, int y) const;
  std::string& operator()(int x, int y);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};

template <typename T>
class uiVector3 : public uiWidget{
private:
	_Vec<T,3> *value;
	_Vec<T,3> last_value;
	Vector2 range;
	std::string text;
	float delta{1.0f};
	int selecting_idx{-1};
	int hovering_idx{-1};
  int start_mouse_pos_x;
	Vector2d _num_display_pos{0, 0};
	_Vec<T, 3> start_mouse_val;
	// float last_value, min_value, max_value, delta_value;
public:
    uiVector3(std::string text_, _Vec<T, 3> *value_, Vector2 range_, const double delta=1.0);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};

class uiCol : public uiWidget{
private:
  Vector3b *value;
  Vector3b last_value;
	std::string text;
	int selecting_idx{-1};
  int start_mouse_pos_x;
	Vector2d _num_display_pos{0, 0};
	Vector3 start_mouse_val;
	// float last_value, min_value, max_value, delta_value;
public:
  uiCol(const std::string &text_, Vector3b *value_);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};

class uiCol2 : public uiWidget{
private:
  Vector3b *value;
  Vector3 hsv;
  Vector3b last_value;
	std::string text;
	Vector2d _num_display_pos{0, 0};
	Vector3 start_mouse_val;
	// float last_value, min_value, max_value, delta_value;
public:
  uiCol2(const std::string &text_, Vector3b *value_);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};

template <typename T>
class uiRange : public uiWidget{
private:
	_Vec<T,2> *value;
	_Vec<T,2> last_value;
	Vector2 range;
	std::string text;
	float delta{1.0f};
	int selecting_idx{-1};
   int start_mouse_pos_x;
public:
    uiRange(const std::string &text_, _Vec<T, 2> *value_, const Vector2 &range_, const double delta=1.0);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};


class uiCollapse : public uiWidget{
    std::string title;
public:
	uiCollapse() = delete;
	uiCollapse(const uiCollapse&) = delete;
	uiCollapse& operator=(const uiCollapse&) = delete;
    
    uiCollapse(const std::string title_);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};

class uiFrame : public uiWidget{
  std::string title;
  int last_not_collapse_size{-1};
  bool last_collapsing{false};
public:
	uiFrame() = delete;
	uiFrame(const uiFrame&) = delete;
	uiFrame& operator=(const uiFrame&) = delete;
    
  uiFrame(const std::string title_);
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};

class uiTextTexture : public uiWidget{
  int last_not_collapse_size{-1};
  bool last_collapsing{false};
public:
	uiTextTexture();
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};


namespace Widget{
	inline uiWidget *Label(const std::string text_)                                  { return (new vkUI::uiLabel(text_));}
	inline uiWidget *Label(const std::string text_, const Vector3b &col)             { return (new vkUI::uiLabel(text_, col));}
	inline uiWidget *Button(const std::string text_, bool *v)                        { return (new vkUI::uiButton(text_, v));}
	inline uiWidget *Button(const std::string text_, std::function<void(void)> f)    { return (new vkUI::uiButtonFunc(text_, f));}
	inline uiWidget *Checkbox(const std::string text_, bool *v)                      { return (new vkUI::uiCheckbox(text_, v));}
	template <typename T>
	inline uiWidget *Slider(const std::string text_, T *v, const Vector2 &r)         { return (new vkUI::uiSliderH<T>(text_, v, r));}
	template <typename T>
	inline uiWidget *Slider(const std::string text_, T *v, const Vector2 &r, const double delta)     { return (new vkUI::uiSliderH<T>(text_, v, r, delta));}
	inline uiWidget *MultiLine(const std::string text_)                              { return (new vkUI::uiMultiLine(text_));}
	inline uiWidget *Table(const std::string &text_)                                 { return (new vkUI::uiTable(text_));}
	inline uiWidget *Table(const std::vector<std::string> text_, const int n)        { return (new vkUI::uiTable(text_, n));}
	/* inline uiWidget *DragVec3(const std::string text_, Vector3 *v, const Vector2 &r) { return (new vkUI::uiVector3(text_, v, r));} */
	template <typename T>
	inline uiWidget *DragVec3(const std::string text_, _Vec<T, 3> *v, const Vector2 &r, const double delta) { return (new vkUI::uiVector3(text_, v, r, delta));}
	template <typename T>
	inline uiWidget *DragVec3(const std::string text_, _Vec<T, 3> *v, const Vector2 &r){ return (new vkUI::uiVector3(text_, v, r));}
	template <typename T> 
	inline uiWidget *Range(const std::string text_, _Vec<T, 2> *v, const Vector2 &r) { return (new vkUI::uiRange(text_, v, r));}
	template <typename T>
	inline uiWidget *Range(const std::string text_, _Vec<T, 2> *v, const Vector2 &r, const double delta) { return (new vkUI::uiRange(text_, v, r, delta));}

	inline uiWidget *Col(const std::string &text_, Vector3b *c) { return (new vkUI::uiCol(text_, c));}
	inline uiWidget *Collapse(const std::string text_)                               { return (new vkUI::uiCollapse(text_));}
	inline uiWidget *Frame(const std::string text_)                                  { return (new vkUI::uiFrame(text_));}
}

} // namespace vkUI
