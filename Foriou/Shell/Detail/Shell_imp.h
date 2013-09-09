#ifndef FORIOU_SHELL_IMP_H_
#define FORIOU_SHELL_IMP_H_

#include "Shell/Shell.h"
#include "Util/Utility.h"
#include "Util/Animator.h"
#include "Shell/Main_win.h"
#include <memory>
#include <map>
#include <vector>

#include <windows.h>
#include <gdiplus.h>

namespace Foriou { namespace Detail {

using namespace Gdiplus;

class Shell_imp : public Event_target, public Animation::IAnimator {
public:
    Shell_imp(const std::string& config);
    virtual ~Shell_imp() throw();

public:
    void register_action(IShell_action* action);

    void show();
    void hide();

public:
    void on_select(Plugin_id pid);

public:
    void set_top();
    void cancel_top();
    bool is_top();

    void minimize();
    void restore();
    bool is_minimal();

public:
    boost::optional<std::wstring>
        require_input(const std::wstring& prompt, Timeout_second timeout);

    void notify_user(const std::wstring& info, Timeout_second timeout);

    boost::optional<bool>
        query_user(const std::wstring& question, Timeout_second timeout);

private:
    bool is_top_;
    bool is_minimal_;

    // implements IAnimator
private:
    virtual bool has_attr(const std::string& name) override;
    virtual void set_attr(const std::string& name, int val) override;
    virtual int get_attr(const std::string& name) override;

private:
    void init_gui_system_();
    void register_handlers_();
    void parse_config_(const std::string& config);
    void init_images_();

private:
    void move_window_(int x, int y);
    void resize_window_(int w, int h);

//! \name   animation
//! @{
private:
    typedef std::unique_ptr<Image> Image_ptr;
    // animation support
    void play_animation_(const std::string& name, std::size_t duration = 0);
    void stop_animation_();

    bool prepare_animation_(const std::string& name);
    void show_frame_(const std::string& name, std::size_t);
    void end_animation_();

    bool is_animating_;
    int animation_counter_;
    std::string buffer_;
    std::vector<Image_ptr> showing_buffer_;
    std::map<std::string, std::vector<std::string>> actions_;
//! @}

private:
    void set_appearance_(Image* img);
//----------------------------------------------------------
//! \name   drag fix
//! @{
private:
    bool prepared_dragging_;
    bool is_dragging_;
    POINT prev_win_pos_;
    POINT prev_cursor_pos_;

    int drag_fix_(WPARAM, LPARAM);

    void prepare_drag_();
    void clear_drag_state_();
    void drag_aux_();
//! @}

//! \name   handlers
//! @{
private:
    // handlers
    void on_drag_();
    void on_drag_end_();

    int on_mouse_move_(WPARAM, LPARAM);
    int on_left_button_down_(WPARAM, LPARAM);
    int on_left_button_up_(WPARAM, LPARAM);
    int on_right_button_down_(WPARAM, LPARAM);
    int on_dbclick_(WPARAM, LPARAM);
    int on_paint_(WPARAM, LPARAM);
    int on_destroy_(WPARAM, LPARAM);
//! @}
//-----------------------------------------------------------
//! \name   representation
//! @{
private:
    HWND win_;

private:
    Image* current_img_;

    // Image buffers
    Image_ptr default_img_;

    std::string default_;

private:
    std::unique_ptr<Main_win> main_win_;
    std::unique_ptr<IShell_action> action_;
//! @}
};

}}  // of namespace Foriou::Detail

#endif // !FORIOU_SHELL_IMP_H_
