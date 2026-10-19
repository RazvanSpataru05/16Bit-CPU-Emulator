#pragma once

#include "Assembler/Assembler.h"

#include "Core/Emulator.h"

#include "UI/UIEditor.h"
#include "UI/AppConfig.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class Application
{
public:
	explicit Application();

	void Init();
	void ProcessEvents();
	void Update();
	void RenderUI();
	void Run();

	void ManageKeyStrokes(const std::optional<sf::Event>& event);

private:
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

private:
	void Reset();

private:
	std::unique_ptr<Emulator> m_emulator;
	std::unique_ptr<Dissasembler> m_dissasembler;

	UIEditor::Mode m_editorMode;

	AppConfig m_appConfig;

	sf::RenderWindow m_window;
	sf::Clock m_deltaClock;
	sf::Clock m_instructionCycle;
	sf::Font m_font;
};