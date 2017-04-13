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

class AbstractCoordinateAction : public InputAction {
    public:
    AbstractCoordinateAction(const IntVector& coordinate);
    virtual ~AbstractCoordinateAction() = 0;
    const IntVector& getCoordinate() const;
    private:
    const IntVector coordinate;
};

class ParticleSelectionAction : public AbstractCoordinateAction {
    public:
    ParticleSelectionAction(const IntVector& coordinate);
};

class TargetSettingAction : public AbstractCoordinateAction {
    public:
    TargetSettingAction(const IntVector& coordinate);
};

}

#endif
