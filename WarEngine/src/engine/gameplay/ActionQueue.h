#pragma once

#include <queue>

#include "engine/gameplay/Action.h"

namespace war
{
    class ActionQueue
    {
    public:
        void push(const Action& action)
        {
            m_queue.push(action);
        }

        [[nodiscard]] bool hasActions() const
        {
            return !m_queue.empty();
        }

        Action pop()
        {
            Action action = m_queue.front();
            m_queue.pop();
            return action;
        }

    private:
        std::queue<Action> m_queue;
    };
}
