#include "Shell/Detail/Shell_imp.h"
#include "Core/Log.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// fix max
#ifdef max
#undef max
#endif // max

namespace Foriou {
namespace Detail {
namespace {

template <class String_type>
inline void log(Log_level lev, String_type&& info)
{
    do_log(L"Shell", lev, std::forward<String_type>(info));
}

}  // of namespace unnamed

void register_window(const std::wstring& name)
{
    static bool first_enter = true;
    if (first_enter) {
        first_enter = false;

        WNDCLASS     wndclass;

        wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wndclass.lpfnWndProc   = event_router;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = 0;
        wndclass.hInstance     = GetModuleHandle(nullptr);
        wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        wndclass.lpszMenuName  = nullptr;
        wndclass.lpszClassName = name.c_str();

        RegisterClass(&wndclass);
    }
}

//! \brief  bad code!
inline std::wstring wstr(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}

const auto shell_name = L"FORIOU_SHELL_IMP";

//! initialize the shell object: load resource, create related UI.
//! \throw  std::runtime_error if initialization failed.
Shell_imp::Shell_imp(const std::string& config)
    : is_top_(),
      is_minimal_(),
      is_animating_(),
      animation_counter_(),
      prepared_dragging_(),
      is_dragging_(),
      prev_win_pos_(),
      prev_cursor_pos_(),
      win_(),
      current_img_()
{
    log(trace, "Init Shell");
    Detail::register_window(shell_name);

    parse_config_(config);

    init_images_();
    register_handlers_();

    log(trace, "Create Shell");
    win_ = ::CreateWindowEx(
            WS_EX_TOOLWINDOW,
            shell_name,
            nullptr,
            WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT,
            current_img_->GetWidth(),
            current_img_->GetHeight(),
            NULL,
            NULL,
            GetModuleHandle(nullptr),
            this
        );

    if (win_ == NULL) {
        throw std::runtime_error("Cannot create shell");
    }
    this->set_top();

    auto logger = [](Log_level lv, const std::wstring& info){
        log(lv, info);
    };
    main_win_.reset(new Main_win(this, logger));
    main_win_->Create();
    if (main_win_->GetHWND() == NULL) {
        throw std::runtime_error("Cannot create main window");
    }

    init_gui_system_();
}

//! if this one is executed, the shell must have been properly
//! initialized.
Shell_imp::~Shell_imp() throw()
{
    assert(::IsWindow(win_));
    DestroyWindow(win_);
    assert(win_ == NULL);
    assert(main_win_ == NULL);
}

void Shell_imp::init_images_()
{
    log(trace, "Load image resource");

    // load the main image
    default_img_.reset(Image::FromFile(Detail::wstr(default_).c_str()));
    if (default_img_->GetLastStatus() != Ok) {
        throw std::runtime_error("Shell: cannot find shell image");
    }

    current_img_ = default_img_.get();
    assert(current_img_->GetWidth() != 0);
    assert(current_img_->GetHeight() != 0);
}

void Shell_imp::init_gui_system_()
{
    assert(::IsWindow(win_));

    const auto transparent_color = RGB(255, 255, 255);

    //设置系统参数，实现拖动时窗体边框透明
    SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, TRUE, NULL, 0);

    SetWindowLong(win_, GWL_EXSTYLE, GetWindowLong(win_, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(win_, transparent_color, NULL, 1);
}

void Shell_imp::register_handlers_()
{
    // WM_CAPTURECHANGE will be called even you don't release it
    // such as alt + tab
    this->add_handler(
        WM_MOUSEMOVE,
        [&](WPARAM wpm, LPARAM lpm){ return on_mouse_move_(wpm, lpm); }
        );
    this->add_handler(
        WM_CAPTURECHANGED,
        [&](WPARAM wpm, LPARAM lpm){ return drag_fix_(wpm, lpm); }
        );
    this->add_handler(
        WM_LBUTTONDOWN,
        [&](WPARAM wpm, LPARAM lpm){ return on_left_button_down_(wpm, lpm); }
        );
    this->add_handler(
        WM_LBUTTONUP,
        [&](WPARAM wpm, LPARAM lpm){ return on_left_button_up_(wpm, lpm); }
        );
    this->add_handler(
        WM_RBUTTONDOWN,
        [&](WPARAM wpm, LPARAM lpm){ return on_right_button_down_(wpm, lpm); }
        );
    this->add_handler(
        WM_LBUTTONDBLCLK,
        [&](WPARAM wpm, LPARAM lpm){ return on_dbclick_(wpm, lpm); }
        );
    this->add_handler(
        WM_PAINT,
        [&](WPARAM wpm, LPARAM lpm){ return on_paint_(wpm, lpm); }
        );
    this->add_handler(
        WM_CLOSE,
        [&](WPARAM wpm, LPARAM lpm){ return ShowWindow(win_, SW_HIDE); }
        );
    this->add_handler(
        WM_DESTROY,
        [&](WPARAM wpm, LPARAM lpm){ return on_destroy_(wpm, lpm); }
        );
}

void Shell_imp::parse_config_(const std::string& config_file)
try {
    log(trace, "Parse config file");

    using namespace boost::property_tree;
    ptree config;
    xml_parser::read_xml(config_file, config, xml_parser::trim_whitespace);

    default_ = config.get<std::string>("config.display");

    for (auto& v: config.get_child("config.actions")) {
        std::vector<std::string> frames;
        auto& action = v.second;
        for (auto& frame: action) {
            frames.push_back(frame.second.data());
        }
        actions_.insert(std::make_pair(v.first, frames));
    }
}
catch (boost::property_tree::xml_parser::xml_parser_error&) {
    throw std::runtime_error("Shell: config file is not availabe or invalid");
}
catch (boost::property_tree::ptree_bad_path&) {
    throw std::runtime_error("Shell: config file format error");
}
//----------------------------------------------------------------------------
void Shell_imp::show()
{
    assert(::IsWindow(win_));

    log(trace, "Show shell");
    ::ShowWindow(win_, SW_SHOW);
}

void Shell_imp::hide()
{
    assert(::IsWindow(win_));

    log(trace, "Hide shell");
    ::ShowWindow(win_, SW_HIDE);
}

void Shell_imp::set_top()
{
    assert(::IsWindow(win_));
    if (!this->is_top()) {
        log(trace, "Set top");
        ::SetWindowPos(win_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        is_top_ = true;
    }
    assert(this->is_top());
}

void Shell_imp::cancel_top()
{
    assert(::IsWindow(win_));
    if (this->is_top()) {
        log(trace, "Cancel top");
        ::SetWindowPos(win_, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        is_top_ = false;
    }
    assert(!this->is_top());
}

bool Shell_imp::is_top()
{
    assert(::IsWindow(win_));
    return is_top_;
}

void Shell_imp::on_select(Plugin_id pid)
{
    assert(::IsWindow(win_));
    if (action_) action_->on_select(pid);
}

void Shell_imp::minimize()
{
    assert(::IsWindow(win_));
    if (!this->is_minimal()) {
        log(trace, "Minimize shell");
        this->hide();
        is_minimal_ = true;
    }
    assert(this->is_minimal());
}

void Shell_imp::restore()
{
    assert(::IsWindow(win_));
    if (this->is_minimal()) {
        log(trace, "Restore shell");
        this->show();
        is_minimal_ = false;
    }
    assert(!this->is_minimal());
}

bool Shell_imp::is_minimal()
{
    assert(::IsWindow(win_));
    return is_minimal_;
}
//----------------------------------------------------------------------------
// animation support
bool Shell_imp::has_attr(const std::string& name)
{
    assert(::IsWindow(win_));
    return actions_.find(name) != actions_.end();
}

int Shell_imp::get_attr(const std::string& name)
{
    assert(::IsWindow(win_));
    assert(this->has_attr(name));
    return 0;
}

void Shell_imp::set_attr(const std::string& name, int val)
{
    assert(::IsWindow(win_));
    assert(this->has_attr(name));
    show_frame_(name, val);
}
//----------------------------------------------------------------------------
void Shell_imp::set_appearance_(Image* image)
{
    //BOOST_LOG_TRIVIAL(trace) << "set appearance";
    assert(::IsWindow(win_));
    if (image != current_img_) {
        log(trace, "Set appearance");

        // resize window
        resize_window_(image->GetWidth(), image->GetHeight());

        current_img_ = image;
        InvalidateRect(win_, nullptr, TRUE);
    }
}
//---------------------------------------------------------------------------
void Shell_imp::stop_animation_()
{
    assert(::IsWindow(win_));
    if (is_animating_) {
        log(trace, "Stop animation");

        using namespace Animation;
        animator(this).stop();
        end_animation_();
    }
    assert(!is_animating_);
}

void Shell_imp::play_animation_(const std::string& name, std::size_t duration)
{
    log(trace, "Play animation: " + name);

    assert(::IsWindow(win_));
    using namespace Animation;
    auto fx = animator(this);

    // cancel existing animation
    if (is_animating_) {
        fx.stop();
        is_animating_ = false;
        animation_counter_ = 0;
    }

    // fix argument
    if (duration == 0) duration = std::numeric_limits<std::size_t>::max();

    // load all images used in the animation
    if (prepare_animation_(name)) {
        // launch animation
        log(trace, "start animation: " + name);
        fx.animate(name, std::numeric_limits<int>::max(), duration, [&]{
            end_animation_();
        });

        is_animating_ = true;
    }
}

bool Shell_imp::prepare_animation_(const std::string& name)
{
    // clear counter
    assert(::IsWindow(win_));
    assert(!is_animating_);
    assert(animation_counter_ == 0);

    // load images
    if (buffer_ != name) {
        log(trace, "Load images: " + name);
        auto img_files = actions_.find(name);
        if (img_files == actions_.end()) return false;

        buffer_ = name;
        showing_buffer_.clear();

        for (auto& elem: img_files->second) {
            auto image = Image::FromFile(Detail::wstr(elem).c_str());
            if (image->GetLastStatus() != Ok) {
                // if any one load fail, clear all(cause the buffer is incomplete)
                buffer_.clear();
                showing_buffer_.clear();
                return false;
            }
            else {
                showing_buffer_.emplace_back(image);
            }
        }
    }
    else {
        log(trace, "Using buffered images");
    }

    return true;
}

void Shell_imp::show_frame_(const std::string& name, std::size_t)
{
    assert(::IsWindow(win_));
    assert(is_animating_);
    assert(!buffer_.empty());
    assert(!showing_buffer_.empty());

    const auto animation_rate = 10;
    auto index = (animation_counter_++ / animation_rate) % showing_buffer_.size();
    set_appearance_(showing_buffer_[index].get());
}

void Shell_imp::end_animation_()
{
    log(trace, "End animation");
    assert(is_animating_);
    assert(::IsWindow(win_));

    animation_counter_ = 0;

    // set to default picture
    set_appearance_(default_img_.get());

    is_animating_ = false;
}
//----------------------------------------------------------------------------
void Shell_imp::move_window_(int x, int y)
{
    assert(::IsWindow(win_));
    if (x < 0 || y < 0) {
        if (x < 0 && y < 0) return;
        RECT rect = {};
        ::GetWindowRect(win_, &rect);
        x < 0? x = rect.left: y = rect.top;
    }
    ::SetWindowPos(win_, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void Shell_imp::resize_window_(int w, int h)
{
    assert(::IsWindow(win_));
    if (w < 0 || h < 0) {
        if (w < 0 && h < 0) return;
        RECT rect = {};
        ::GetWindowRect(win_, &rect);
        w < 0?
        w = (rect.right - rect.left):
        h = (rect.bottom - rect.top);
    }
    ::SetWindowPos(win_, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE);
}
//----------------------------------------------------------------------------
// drag fix
int Shell_imp::drag_fix_(WPARAM wpm, LPARAM lpm)
{
    assert(::IsWindow(win_));
    if (win_ != reinterpret_cast<HWND>(lpm)) {
        prepared_dragging_ = false;

        if (is_dragging_) {
            on_drag_end_();
        }

        is_dragging_ = false;
    }
    return 0;
}

void Shell_imp::clear_drag_state_()
{
    assert(::IsWindow(win_));
    ::ReleaseCapture();
}

void Shell_imp::prepare_drag_()
{
    assert(::IsWindow(win_));
    assert(!prepared_dragging_);
    assert(!is_dragging_);
    if (GetCursorPos(&prev_cursor_pos_)) {
        RECT rect = {};
        ::GetWindowRect(win_, &rect);
        prev_win_pos_.x = rect.left;
        prev_win_pos_.y = rect.top;
        prepared_dragging_ = true;
        ::SetCapture(win_);
    }
}

void Shell_imp::drag_aux_()
{
    assert(::IsWindow(win_));
    POINT pt = {};
    if (::GetCursorPos(&pt)) {
        if (!is_dragging_) {
            is_dragging_ = true;
            on_drag_();
        }
        int win_x = prev_win_pos_.x + (pt.x - prev_cursor_pos_.x);
        int win_y = prev_win_pos_.y + (pt.y - prev_cursor_pos_.y);
        move_window_(win_x, win_y);
    }
}

//-----------------------------------------------------------------
// handlers
void Shell_imp::on_drag_()
{
    assert(::IsWindow(win_));
    play_animation_("drag");
    if (action_) action_->on_drag();
}

void Shell_imp::on_drag_end_()
{
    assert(::IsWindow(win_));
    assert(is_animating_);
    stop_animation_();
}

int Shell_imp::on_paint_(WPARAM wpm, LPARAM lpm)
{
    assert(::IsWindow(win_));
    PAINTSTRUCT ps = {};
    HDC dc = BeginPaint(win_, &ps);

    const auto w = current_img_->GetWidth();
    const auto h = current_img_->GetHeight();

    // draw the window content
    Graphics g(dc);
    Rect r(0, 0, w, h);
    g.DrawImage(current_img_, r);

    EndPaint(win_, &ps);
    return 0;
}

int Shell_imp::on_mouse_move_(WPARAM wp, LPARAM lp)
{
    //BOOST_LOG_TRIVIAL(trace) << "Event: mouse move";
    assert(::IsWindow(win_));
    if (prepared_dragging_) drag_aux_();
    return 0;
}

int Shell_imp::on_left_button_down_(WPARAM wp, LPARAM lp)
{
    log(trace, "Event: left button down");
    assert(::IsWindow(win_));

    prepare_drag_();
    if (action_) action_->on_click();

    return 0;
}

int Shell_imp::on_left_button_up_(WPARAM wp, LPARAM lp)
{
    log(trace, "Event: left button up");
    assert(::IsWindow(win_));

    clear_drag_state_();
    return 0;
}

int Shell_imp::on_right_button_down_(WPARAM wp, LPARAM lp)
{
    assert(::IsWindow(win_));

    log(trace, "Event: right button down");

    //play_animation_("shy", 1000);

    if (action_) action_->on_right_click();

    return 0;
}

int Shell_imp::on_dbclick_(WPARAM wp, LPARAM lp)
{
    log(trace, "Event: double click");

    assert(::IsWindow(win_));

    main_win_->AnimatedShowWindow(500, AW_CENTER);
    play_animation_("smile", 1000);
    if (action_) action_->on_dbclick();
    return 0;
}

int Shell_imp::on_destroy_(WPARAM wp, LPARAM lp)
{
    log(trace, "Destroy Shell");
    assert(::IsWindow(win_));

    win_ = NULL;
    main_win_.reset();
    action_.reset();

    return 0;
}

//--------------------------------------------------------------
void Shell_imp::register_action(IShell_action* action)
{
    action_.reset(action);
}

//---------------------------------------------------------------
boost::optional<std::wstring>
    Shell_imp::require_input(const std::wstring& prompt, Timeout_second timeout)
{
    return boost::none;
}

void Shell_imp::notify_user(const std::wstring& info, Timeout_second timeout)
{
}

boost::optional<bool>
    Shell_imp::query_user(const std::wstring& question, Timeout_second timeout)
{
    return boost::none;
}

}}  // of namespace Foriou::Detail

//--------------------------------------------------------
// End of File
//--------------------------------------------------------
