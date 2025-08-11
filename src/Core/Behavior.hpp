#ifndef BEHAVIOR_H_
#define BEHAVIOR_H_
#include "StateCommon.h"
#include "FrameTimer.h"
#include "EnumMapping.hpp"
#include <iostream>

namespace Behavior
{
    class NodeBase;
}

/*struct ComponentAI
{
    std::unique_ptr<Behavior::NodeBase> m_logic;
    std::optional<CharState> m_requestedState;
    ORIENTATION m_requestedOrientation = ORIENTATION::UNSPECIFIED;
    Vector2<float> m_navigationTarget;
    bool m_isNavigating = false;
    float m_additionalAccel = 0.0f;
    entt::entity m_chaseTarget;
    bool m_allowLeaveState = true;
};*/

namespace Behavior
{
    enum class Status {
        SUCCESS,
        FAILURE,
        RUNNING
    };

    struct Context
    {
        
    };

    class NodeBase
    {
    public:
        NodeBase(const std::string &name_);

        virtual void enter() = 0;
        virtual void leave();

        virtual Status update() = 0;

        virtual std::string stringify(size_t intend_) const;

    protected:
        virtual std::string describeSelf() const;

        const std::string m_name;
    };

    template<typename... Ts>
    class Sequence : public NodeBase
    {
    public:
        Sequence(const std::string &name_, Ts&&... elems_) :
            NodeBase(name_)
        {
            const auto sz = sizeof... (Ts);
            std::cout << "Creating sequence \"" << describeSelf() << "\" for " << sz << std::endl;

            size_t iter = 0;
            
            ([&]
            {
                m_nodes[iter] = std::make_unique<Ts>(std::forward<Ts>(elems_));
                ++iter;
            } (), ...);
        }

        virtual void enter() override
        {
            std::cout << "\"" << describeSelf() << "\" - enter" << std::endl;
            m_it = m_nodes.begin();
            (*m_it)->enter();
        }

        virtual void leave() override
        {
            std::cout << "\"" << describeSelf() << "\" - leaving" << std::endl;
        }

        virtual Status update() override
        {
            std::cout << "\"" << describeSelf() << "\" - update" << std::endl;
            while (m_it != m_nodes.end())
            {
                auto res = (*m_it)->update();
                if (res == Status::SUCCESS)
                {
                    (*m_it)->leave();
                    m_it++;
                    if (m_it != m_nodes.end())
                        (*m_it)->enter();
                    else
                    {
                        std::cout << "\"" << describeSelf() << "\" - reporting " << serialize(Status::SUCCESS) << std::endl;
                        return Status::SUCCESS;
                    }
                }
                else if (res == Status::FAILURE)
                {
                    (*m_it)->leave();
                    std::cout << "\"" << describeSelf() << "\" - reporting " << serialize(res) << std::endl;
                    return res;
                }
                else
                {
                    std::cout << "\"" << describeSelf() << "\" - reporting " << serialize(res) << std::endl;
                    return res;
                }
            }

            throw std::runtime_error("Loop condition contradicts internal check");
        }

        virtual std::string stringify(size_t intend_) const override
        {
            std::string res(intend_, ' ');
            res += describeSelf() + "\n";
            for (const auto &el : m_nodes)
                res += el->stringify(intend_ + 1);

            return res;
        }

    protected:
        virtual std::string describeSelf() const
            {
                return m_name + " (size=" + std::to_string(m_nodes.size()) + ")";
            }

        std::array<std::unique_ptr<NodeBase>, sizeof...(Ts)> m_nodes;
        decltype(m_nodes)::iterator m_it;
    };

    class RequestState : public NodeBase
    {
    public:
        template<typename T>
        RequestState(const T &state_) :
            NodeBase("Request " + serialize(state_)),
            m_state(static_cast<CharState>(state_))
        {
            std::cout << "Creating \"" << describeSelf() << "\"" << std::endl;
        }

        // TODO: pass context
        virtual void enter() override
        {
            std::cout << "\"" << describeSelf() << "\" - enter" << std::endl;
            // TODO:
        }

        virtual void leave() override
        {
            std::cout << "\"" << describeSelf() << "\" - leaving" << std::endl;
        }

        virtual Status update() override
        {
            std::cout << "\"" << describeSelf() << "\" - update" << std::endl;
            // TODO:
        }

        virtual std::string stringify(size_t intend_) const override
        {
            std::string res(intend_, ' ');
            res += describeSelf() + "\n";

            return res;
        }

    protected:
        virtual std::string describeSelf() const
            {
                //return m_name + " (size=" + std::to_string(m_nodes.size()) + ")"; TODO:
                return "";
            }

        const CharState m_state;
    };

}

SERIALIZE_ENUM(Behavior::Status, {
    ENUM_AUTO(Behavior::Status, SUCCESS),
    ENUM_AUTO(Behavior::Status, FAILURE),
    ENUM_AUTO(Behavior::Status, RUNNING)
});

#endif
