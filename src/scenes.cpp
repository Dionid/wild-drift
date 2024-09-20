#include "scenes.h"

void MainMenuScene::Init() {
    EnableCursor();

    this->scpMultiplayer->Deinit();

    // ## MainMenu
    MainMenu* mainMenu = this->nodeStorage->AddNode(std::make_unique<MainMenu>());

    // # Events
    // ## StartEvent
    auto onStartEvent = [
        this
    ](const cen::Event* event) {
        this->eventBus.Emit(
            std::make_unique<cen::SceneChangeRequested>(
                LocalMatchSceneName
            )
        );
    };

    this->eventBus.On(
        StartEvent{},
        std::make_unique<cen::EventListener>(
            onStartEvent
        )
    );

    // ## HostEvent
    auto onHostEvent = [
        this
    ](const cen::Event* event) {
        this->sceneManager->ChangeScene(
            std::make_unique<ServerLobbyScene>(
                this->scpMultiplayer,
                this->gameAudio,
                this->screen,
                this->camera,
                this->renderingEngine,
                this->eventBus.parent
            )
        );
    };

    this->eventBus.On(
        HostEvent{},
        std::make_unique<cen::EventListener>(
            onHostEvent
        )
    );

    // ## JoinEvent
    auto onJoinEvent = [
        this
    ](const cen::Event* event) {
        this->sceneManager->ChangeScene(
            std::make_unique<ClientLobbyScene>(
                this->scpMultiplayer,
                this->gameAudio,
                this->screen,
                this->camera,
                this->renderingEngine,
                this->eventBus.parent
            )
        );
    };

    this->eventBus.On(
        JoinEvent{},
        std::make_unique<cen::EventListener>(
            onJoinEvent
        )
    );
}