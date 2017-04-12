#include "InputAction.hpp"

namespace wotmin2d {

InputAction::~InputAction() {}

ExitAction::~ExitAction() {}

MouseDownAction::MouseDownAction(const IntVector& coordinate):
    coordinate(coordinate) {}

MouseDownAction::~MouseDownAction() {}

const IntVector& MouseDownAction::getCoordinate() const {
    return coordinate;
}

}
