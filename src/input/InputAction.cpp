#include "InputAction.hpp"

namespace wotmin2d {

InputAction::~InputAction() {}

ExitAction::~ExitAction() {}

AbstractCoordinateAction::AbstractCoordinateAction(const IntVector& coordinate):
    coordinate(coordinate) {}

AbstractCoordinateAction::~AbstractCoordinateAction() {}

const IntVector& AbstractCoordinateAction::getCoordinate() const {
    return coordinate;
}

ParticleSelectionAction::ParticleSelectionAction(const IntVector& coordinate):
    AbstractCoordinateAction(coordinate) {}

TargetSettingAction::TargetSettingAction(const IntVector& coordinate):
    AbstractCoordinateAction(coordinate) {}


}
