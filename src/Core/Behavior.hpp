#ifndef BEHAVIOR_H_
#define BEHAVIOR_H_
#include "FrameTimer.h"
#include "EnumMapping.hpp"
#include <iostream>

namespace Behavior
{
    enum class Status {
        SUCCESS,
        FAILURE,
        RUNNING
    };

    class NodeBase
    {
    public:
        NodeBase(const std::string &name_) :
            m_name(name_)
        {
        }

        virtual void enter() = 0;
        virtual void leave() {}

        virtual Status update() = 0;

        virtual std::string stringify(int intend_) const
        {
            return std::string(intend_, ' ') + describeSelf() + "\n";
        }

    protected:
        virtual std::string describeSelf() const
        {
            return m_name;
        }

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

        virtual std::string stringify(int intend_) const override
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

    class WaitAndRand : public NodeBase
    {
    public:
        WaitAndRand(uint32_t frames_) :
            NodeBase("WaitAndRand"),
            m_timer(frames_)
        {
            std::cout << "Creating \"" << describeSelf() << "\" for " << frames_ << " frames" << std::endl;
        }

        virtual void enter() override
        {
            std::cout << "\"" << describeSelf() << "\" - enter" << std::endl;
            m_timer.setCurrentFrame(0);
        }

        virtual void leave() override
        {
            std::cout << "\"" << describeSelf() << "\" - leaving" << std::endl;
        }

        virtual Status update() override
        {
            if (m_timer.update())
            {
                auto rng = rand() % 2;
                auto res = rng ? Status::SUCCESS : Status::FAILURE;
                std::cout << "\"" << describeSelf() << "\" - reporting " << serialize(res) << std::endl;
                return res;
            }
            else
            {
                std::cout << "\"" << describeSelf() << "\" - update" << std::endl;
                return Status::RUNNING;
            }
        }

    protected:
        virtual std::string describeSelf() const override
        {
            return NodeBase::describeSelf() + " (duration=" + std::to_string(m_timer.getDuration()) + ")";
        }

        FrameTimer<true> m_timer;
    };

}

SERIALIZE_ENUM(Behavior::Status, {
    ENUM_AUTO(Behavior::Status, SUCCESS),
    ENUM_AUTO(Behavior::Status, FAILURE),
    ENUM_AUTO(Behavior::Status, RUNNING)
});

#endif
