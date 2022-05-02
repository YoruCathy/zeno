#include <zeno/zeno.h>
#include <zeno/utils/logger.h>
#include <zeno/types/ListObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/extra/GlobalComm.h>
#include <zeno/utils/cppdemangle.h>
#include <zeno/core/Graph.h>

namespace zeno {
namespace {

struct ToView : zeno::INode {
    virtual void complete() override {
        log_debug("ToView: {}", myname);
        graph->nodesToExec.insert(myname);
    }

    bool hasStaticViewed = true;

    virtual void apply() override {
        auto p = get_input("object");
        if (!p) {
            log_error("ToView: given object is nullptr");
        } else {
            std::shared_ptr<IObject> pp;
            if (p->userData().has("static")) {
                pp = p->clone();
            } else {
                pp = p->clone();
            }
            if (!pp) {
                log_warn("ToView: given object doesn't support clone, giving up");
            } else {
                log_debug("ToView: added view object of type {}", cppdemangle(typeid(*p)));
                /* pp->userData().set("nodeid", objectFromLiterial(this->myname)); */
                getThisSession()->globalComm->addViewObject(this->myname, std::move(pp));
            }
        }
        set_output("object", std::move(p));
    }
};

ZENDEFNODE(ToView, {
    {"object"},
    {"object"},
    {},
    {"graphtool"},
});

struct HelperMute : zeno::INode {
    virtual void apply() override {
        for (auto const &[name, _]: this->inputs) {
            if (name == "SRC") continue;//sk
            set_output(name, get_input(name));
        }
    }
};

ZENDEFNODE(HelperMute, {
    {},
    {},
    {{"string", "NOTE", "Dont-use-this-node-directly"}},
    {"graphtool"},
});

struct HelperOnce : zeno::INode {
    bool m_done = false;

    virtual void preApply() override {
        if (!m_done) {
            INode::preApply();
            m_done = true;
        }
    }

    virtual void apply() override {
        for (auto const &[name, _]: this->inputs) {
            if (name == "SRC") continue;//sk
            set_output(name, get_input(name));
        }
    }
};

ZENDEFNODE(HelperOnce, {
    {},
    {},
    {{"string", "NOTE", "Dont-use-this-node-directly"}},
    {"graphtool"},
});

}
}
