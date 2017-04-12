#ifndef INPUTACTION_HPP
#define INPUTACTION_HPP

#include "../game/Vector.hpp"

namespace wotmin2d {

class InputAction {
    public:
    virtual ~InputAction() = 0;
};

class ExitAction : public InputAction {
    public:
    virtual ~ExitAction();
};

class MouseDownAction : public InputAction {
    public:
    MouseDownAction(const IntVector& coordinate);
    virtual ~MouseDownAction();
    const IntVector& getCoordinate() const;
    private:
    const IntVector coordinate;
};

}

#endif
