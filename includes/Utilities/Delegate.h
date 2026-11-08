#pragma once


#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>


using SubscriptionId = uint32_t;
constexpr SubscriptionId InvalidSubscription = UINT32_MAX;

template <typename... Params>
struct DelegateState {
    struct Slot {
        std::function<void(Params...)> func;
        // Flip switch for unbinding
        bool active {true};
    };

    // Bind order
    std::vector<std::pair<SubscriptionId, std::shared_ptr<Slot>>> slots{};
    SubscriptionId next = 0;

    SubscriptionId add(std::function<void(Params...)> fn) {
        const SubscriptionId id = ++next;
        slots.emplace_back(id, std::make_shared<Slot>(std::move(fn)));
        return id;
    }

    void remove(SubscriptionId id) {
        auto it = std::find_if(slots.begin(), slots.end(), [id](const auto& s) {
            return s.first == id;
        });
        if (it == slots.end()) return;
        it->second->active = false;
        slots.erase(it);
    }
};

template <typename... Params>
class ScopedBinding {
    std::weak_ptr<DelegateState<Params...>> m_state;
    SubscriptionId m_id = InvalidSubscription;

public:
    ScopedBinding() = default;
    ScopedBinding(std::weak_ptr<DelegateState<Params...>> state, SubscriptionId id)
    :   m_state(std::move(state)), m_id(id){}

    ScopedBinding(const ScopedBinding&) = delete;
    ScopedBinding& operator=(const ScopedBinding&) = delete;

    ScopedBinding(ScopedBinding&& sb) noexcept :
        m_state(std::move(sb.m_state)), m_id(std::exchange(sb.m_id, InvalidSubscription)){}

    ScopedBinding& operator=(ScopedBinding&& sb) noexcept {
        if (this != &sb) {
            unbind();
            m_state = std::move(sb.m_state);
            m_id = std::exchange(sb.m_id, InvalidSubscription);
        }
        return *this;
    }

    ~ScopedBinding() { unbind();}

    void unbind() {
        if (auto s = m_state.lock()) {
            s->remove(m_id);
        }
        m_state.reset();
        m_id = InvalidSubscription;
    }

    bool isBound() const {
        return m_id != InvalidSubscription && !m_state.expired();
    }
};

template <typename... Params>
class Delegate {
    using State = DelegateState<Params...>;

    std::shared_ptr<State> m_state = std::make_shared<State>();

public:
    Delegate() = default;
    Delegate(const Delegate&) = delete;
    Delegate& operator=(const Delegate&) = delete;

    // Bindings follow state so move should keep them valid
    Delegate(Delegate&& d) noexcept:
        m_state(std::exchange(d.m_state, std::make_shared<State>())){}

    Delegate& operator=(Delegate&& d) noexcept {
        if (this != &d) {
            m_state = std::exchange(d.m_state, std::make_shared<State>());
        }

        return *this;
    }

    SubscriptionId bindFunction(std::function<void(Params...)> fn) {
        if (!fn) {
           std::cerr << "DELEGATE: Cannot bind null function.\n";
           return InvalidSubscription;
        }

        return m_state->add(std::move(fn));
    }

    // Unscoped ver, needs unbinding before the owning object is dead
    template <class Owner>
    SubscriptionId bindFunction(Owner* owner, void (Owner::*func)(Params...)) {
        return bindFunction([owner, func](Params... p) {
            (owner->*func)(std::forward<Params>(p)...);
        });
    }

    ScopedBinding<Params...> bindScoped(std::function<void(Params...)> func) {
        return ScopedBinding<Params...>(m_state, bindFunction(std::move(func)));
    }


    template <class Owner>
    ScopedBinding<Params...> bindScoped(Owner* owner, void (Owner::*func)(Params...)) {
        return ScopedBinding<Params...>(m_state, bindFunction(owner, func));
    }

    void unbind(SubscriptionId id) { m_state->remove(id); }
    void clear() { while (!m_state->slots.empty()) m_state->remove(m_state->slots.back().first); }
    size_t size() const { return m_state->slots.size(); }

    void broadcast(Params... params) const {
        auto state = m_state;
        auto snap = state->slots;
        for (auto& [id, slot] : snap) {
            if (slot->active) {
                slot->func(params...);
            }
        }
    }
};
