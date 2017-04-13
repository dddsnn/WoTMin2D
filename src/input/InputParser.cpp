#include "InputParser.hpp"

namespace wotmin2d {

InputParser::InputParser(): event_buffer(nullptr) {
    event_buffer = new SDL_Event[batch_size];
}

InputParser::~InputParser() {
    delete[] event_buffer;
}

std::vector<std::unique_ptr<InputAction>> InputParser::parseInput() {
    std::vector<std::unique_ptr<InputAction>> actions;
    SDL_PumpEvents();
    // Get rid of anything that isn't a mouse click or key down.
    SDL_FilterEvents(&isMouseOrKeyDownEvent, nullptr);
    addActions(SDL_KEYDOWN, &parseKeyDown, std::back_inserter(actions));
    addActions(SDL_MOUSEBUTTONDOWN, &parseMouseDown,
               std::back_inserter(actions));
    return actions;
}

int InputParser::isMouseOrKeyDownEvent(void*, SDL_Event* event) {
    return event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_KEYDOWN;
}

std::unique_ptr<InputAction> InputParser::parseKeyDown(SDL_Event& event) {
    SDL_KeyboardEvent& key_event = event.key;
    if (key_event.keysym.sym == SDLK_ESCAPE) {
        return std::unique_ptr<InputAction>(new ExitAction);
    }
    return nullptr;
}

std::unique_ptr<InputAction> InputParser::parseMouseDown(SDL_Event& event) {
    SDL_MouseButtonEvent& mouse_event = event.button;
    int x = mouse_event.x;
    int y = mouse_event.y;
    switch (mouse_event.button) {
    case SDL_BUTTON_LEFT:
        return std::unique_ptr<InputAction>(
            new ParticleSelectionAction(IntVector(x, y)));
        break;
    case SDL_BUTTON_RIGHT:
        return std::unique_ptr<InputAction>(
            new TargetSettingAction(IntVector(x, y)));
        break;
    }
    return nullptr;
}

int InputParser::getEvents(std::uint32_t type) {
    return SDL_PeepEvents(event_buffer, batch_size, SDL_GETEVENT, type, type);
}

}
