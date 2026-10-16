#include "UI/Application.h"

Application::Application() :
	m_emulator{ std::make_unique<Emulator>()},
	m_dissasembler{ std::make_unique<Dissasembler>() },
	m_assembler{std::make_unique<Assembler>()},
	m_editorMode{ UIEditor::Mode::EDIT }
{

}

Assembler& Application::GetAssembler()
{
	return *m_assembler;
}

void Application::Init()
{
	const sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	m_window = sf::RenderWindow(sf::VideoMode(desktop), "16-Bit CPU Emulator", sf::Style::Default);
	m_window.setFramerateLimit(60);
	ImGui::SFML::Init(m_window);
}

void Application::ProcessEvents()
{
	while (const std::optional event = m_window.pollEvent())
	{
		ImGui::SFML::ProcessEvent(m_window, event.value());

		if (event->is<sf::Event::KeyPressed>())
		{
			ManageKeyStrokes(event);
		}

		if (event->is<sf::Event::Closed>())
		{
			m_window.close();
		}
	}
}

void Application::Update()
{
	if (m_appConfig.executeAuto && m_instructionCycle.getElapsedTime().asSeconds() >= m_appConfig.autoSpeed)
	{
		m_emulator->GetCPU().Step();
		m_instructionCycle.restart();
	}
	ImGui::SFML::Update(m_window, m_deltaClock.restart());
}

void Application::RenderUI()
{
	m_window.clear(sf::Color(65, 65, 65));

	UIEditor::DrawCPUState(m_emulator->GetCPU());
	UIEditor::DrawAssemblyPanel(m_editorMode, m_emulator->GetCPU().GetMemoryUnit(),
		*m_dissasembler, m_emulator->GetCPU(), *m_assembler, m_emulator->GetTerminal(), m_emulator->GetInterruptController(),
		m_emulator->GetKeyboard(), m_appConfig.executeAuto);

	UIEditor::DrawMemoryView(m_emulator->GetCPU().GetMemoryUnit(),
		m_emulator->GetCPU(), m_appConfig.followPC);

	UIEditor::DrawMenu(m_emulator->GetCPU().GetMemoryUnit(),
		m_appConfig.followPC, m_appConfig.executeAuto, m_emulator->GetCPU());

	UIEditor::DrawSpeedSlider(m_appConfig.autoSpeed);
	UIEditor::DrawHelpMenu();

	ImGui::SFML::Render(m_window);
	m_window.display();
}

void Application::Run()
{
	Init();
	while (m_window.isOpen())
	{
		ProcessEvents();
		Update();
		RenderUI();
	}
	ImGui::SFML::Shutdown();
}

void Application::ManageKeyStrokes(const std::optional<sf::Event>& event)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		return;
	}

	const auto keyCode = event->getIf<sf::Event::KeyPressed>();

	switch (keyCode->code)
	{
	case sf::Keyboard::Key::Escape:
		m_window.close();
		break;

	case sf::Keyboard::Key::Space:
		m_emulator->GetCPU().Step();
		break;

	case sf::Keyboard::Key::R:
		Reset();
		break;

	// Speed Slider Controls
	case sf::Keyboard::Key::D:
	{
		m_appConfig.autoSpeed = std::min(2.00f, m_appConfig.autoSpeed + 0.10f);
		break;
	}
	case sf::Keyboard::Key::A:
	{
		m_appConfig.autoSpeed = std::max(0.10f, m_appConfig.autoSpeed - 0.10f);
		break;
	}

	default:
		break;
	}
}

void Application::Reset()
{
	m_emulator->GetCPU().GetMemoryUnit().RestoreSnapshot();
	m_emulator->GetCPU().Reset();
	m_appConfig.Reset();
}
