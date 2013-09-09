#ifndef FORIOU_PLUGIN_VDESK_IMP_H_
#define FORIOU_PLUGIN_VDESK_IMP_H_

#include <Windows.h>
#include <vector>
#include <functional>
#include <memory>

namespace Foriou { namespace Plugin {

namespace Detail {

template <class Obj>
class Singleton {
public:
    static std::shared_ptr<Obj> get();

protected:
    Singleton()
    {
    }

    ~Singleton() throw()
    {
    }

private:
    /*! I use _*_ notation to represent a
     *  static member.
     */
    static std::shared_ptr<Obj> _inst_;
};

class Singleton_creater {
public:
    template <class Obj>
    static Obj* create() { return new Obj; }
};

template <class Obj>
std::shared_ptr<Obj> Singleton<Obj>::_inst_;

template <class Obj>
std::shared_ptr<Obj> Singleton<Obj>::get()
{
    if (!_inst_) {
        _inst_.reset(Singleton_creater::create<Obj>());
    }
    return _inst_;
}

/*!
 * \brief   state object of a virtual desktop.
 *
 * we can use this object to save and close the current desktop,
 * as well as restore a saved desktop.
 *
 * \note    it's the client's responsibility to guarantee that
 *          that are only one state activiated simultaneousely.
 */
class VDesktop_state {
public:
    /*! used to filter system windows which will not be removed from destkop. */
    typedef std::function<bool(HWND)> filter_functor;

    explicit VDesktop_state(filter_functor filter, bool init_state = false)
        : filter_(filter), is_active_(init_state)
    {
    }

    /*! default destruction. */

    /*!
     * \brief   save and close the current desktop.
     * \post    is_active() == false.
     * \return  true if the deactivation is done; if it's already
     *          disactive, false is returned.
     */
    bool save_and_close();

    /*!
     * \brief   restore the saved state.
     * \post    is_active() == true.
     * \return  true if the activation is done; if it's already
     *          active, false is returned.
     */
    bool restore();

    /*!
     * \brief   destroy current desktop.
     * \pre     is_active() == false;
     * \post    is_active() == false;
     * \throw   std::runtime_error if processes cannot be destroyed.
     */
    void destroy_desktop();

    bool is_active() const { return is_active_; }

private:
    bool is_filtered_(HWND hwnd);

private:
    bool is_active_;
    filter_functor filter_;

	struct Win_state {
		HWND hwnd;
		UINT show_state;
		explicit Win_state(HWND wnd, UINT show) : hwnd(wnd), show_state(show) {}
	};

    std::vector<Win_state> state_;
};

// predefined filters
bool filter_current_process(HWND hwnd);

}  // of namespace Detail

class VDesk : public Detail::Singleton<VDesk> {
public:
    enum { initial_desk = 0 };
    enum { desk_limit = 4 };

public:
    ~VDesk() throw();

    /*!
     * \brief   switch to desktop \a i.
     * \pre     0 <= i && i < 4.
     * \throw   std::runtime_error if creating desktop failed.
     *
     * \note    the app which this plugin resides may automatically
     *          be moved to the new desktop.
     */
    void switch_to(const int i);

private:
    VDesk();
    friend class Detail::Singleton_creater;

private:
	int current_;
    std::vector<Detail::VDesktop_state> desks_;
};

}}  // of namespace Foriou::Plugin

#endif // !FORIOU_PLUGIN_VDESK_IMP_H_
//////////////////////////////////////////
//    End of File
//        Dedicated for myself
//        2013-09-01
//////////////////////////////////////////
