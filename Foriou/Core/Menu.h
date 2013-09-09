
#ifndef FORIOU_MENU_H_
#define FORIOU_MENU_H_

#include "Util/Win_base.h"
#include <map>
#include <functional>

namespace Foriou { namespace Detail {

class Menu : public Win_base {
public:
    typedef unsigned Item_id;
    typedef std::function<void()> Item_handler;

public:
    template <class Menu_item>
    explicit Menu(Menu_item&& items)
        : Win_base(nullptr), items_(std::forward<Menu_item>(items))
    {
        create_self_();
    }

    Menu() : Win_base(nullptr)
    {
        create_self_();
    }

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm) override;
    virtual void Notify(TNotifyUI& notifier) override;

public:
    void show(POINT pt);
    void hide();

private:
    void adjust_position(POINT pt);
    void invoke_handler_(Item_id item);

    void create_self_();

private:
    virtual CControlUI* Init() override;

private:
    std::map<Item_id, Item_handler> items_;
};

}}  // of namespace Foriou::Detail


#endif // FORIOU_MENU_H_
