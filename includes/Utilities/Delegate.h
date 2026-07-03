#pragma once


#include <functional>
#include <vector>
#include <iostream>


template<typename Return, typename... Parameters>
class SingleCastDelegate
{
    std::function<Return(Parameters...)> functionToCall;

public:
    SingleCastDelegate() = default;
    virtual ~SingleCastDelegate() = default;

    virtual void bindFunction(std::function<Return(Parameters...)> func)
    {
        if (!func)
        {
            std::cerr << "DELEGATE::Cannot bind null function pointer. \n";
            return;
        }
        functionToCall = func;
    }

    template <class Owner>
    void bindFunction(Owner* owningObject,Return(Owner::*func)(Parameters...))
    {
        // Lambda to get the full function signature devoid of class type
        bindFunction([owningObject, func](Parameters... p){(owningObject->*func)(p...);});
    }

    virtual void unbindFunction(std::function<Return(Parameters...)> func)
    {
        if (func && &functionToCall != &func)
        {
            functionToCall = nullptr;
        }
    }

    template <class Owner>
    void unbindFunction(Owner* owningObject,Return(Owner::*func)(Parameters...))
    {
        // Lambda to get the full function signature devoid of class type
        unbindFunction([owningObject, func](Parameters... p){(owningObject->*func)(p...);});
    }

    virtual void broadcast(Parameters... p) const
    {
        if (functionToCall) functionToCall(p...);
    }
};

template <typename... Parameters>
class Delegate: public SingleCastDelegate<void, Parameters...>
{
    // Hold function pointers, only single cast delegates should have return types
    std::vector<std::function<void(Parameters...)>> m_functions;

public:
    Delegate() = default;

    void bindFunction(std::function<void(Parameters...)> func) override
    {
        if (!func)
        {
            std::cerr << "DELEGATE::Cannot bind null function pointer. \n";
            return;
        }
        m_functions.push_back(func);
    }

    template <class Owner>
    void bindFunction(Owner* owningObject,void(Owner::*func)(Parameters...))
    {
        // Lambda to get the full function signature devoid of class type
        bindFunction([owningObject, func](Parameters... p){(owningObject->*func)(p...);});
    }

    virtual void unbindFunction(std::function<void(Parameters...)> func) override
    {
        if (!func) return;
        for (size_t i = 0; i < m_functions.size(); ++i)
        {
            if (&func == &m_functions[i])
            {
                m_functions.erase(m_functions.begin() + i);
            }
        }
    }

    template <class Owner>
    void unbindFunction(Owner* owningObject,void(Owner::*func)(Parameters...))
    {
        // Lambda to get the full function signature devoid of class type
        unbindFunction([owningObject, func](Parameters... p){(owningObject->*func)(p...);});
    }

    void broadcast(Parameters... params) const override
    {
        for (auto& f: m_functions)
        {
            if (f) f(params...);
        }
    }
};