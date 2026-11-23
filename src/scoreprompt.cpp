#include "scoreprompt.h"

#include <chrono>
#include <exception>
#include <expected>
#include <format>
#include <future>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "gameconfig.h"
#include "gamedraw.h"
#include "scoreboard.h"
#include "textbox.h"

namespace {
	enum class Phase { Hidden, Ask, EnterName, Waiting, Result };
	using BoardResult = std::expected<std::vector<ScoreEntry>, std::string>;

	constexpr sf::Color TEXT_COLOR{230, 230, 230, 255};
	constexpr sf::Color PANEL_COLOR{14, 14, 14, 255};
	constexpr sf::Color FIELD_COLOR{40, 40, 40, 255};
	constexpr sf::Color CLEAR{0, 0, 0, 0};
	constexpr sf::Color CONFIRM_COLOR{50, 150, 50, 255};
	constexpr sf::Color CANCEL_COLOR{150, 50, 50, 255};
	constexpr sf::Color NEUTRAL_COLOR{70, 70, 70, 255};

	GameState* promptGame = nullptr;

	Phase phase = Phase::Hidden;
	cenui::Frame* panel = nullptr;
	cenui::Frame* title = nullptr;
	cenui::Frame* body = nullptr;
	cenui::Frame* nameField = nullptr;
	cenui::Frame* confirmButton = nullptr;
	cenui::Frame* cancelButton = nullptr;
	std::optional<TextBox> nameBox; // kept between rounds so the name is remembered
	std::future<BoardResult> pending;

	void showButton(cenui::Frame* button, const std::string& label, sf::Color color) {
		button->visible(true)->text(label)->bgColor(color);
	}

	void setPhase(Phase next) {
		phase = next;
		panel->visible(next != Phase::Hidden);
		nameField->visible(next == Phase::EnterName);
		nameBox->setFocused(next == Phase::EnterName);
		confirmButton->visible(false);
		cancelButton->visible(false);
		body->text("");

		switch (next) {
		case Phase::Hidden:
			break;
		case Phase::Ask:
			title->text("YOUR SCORE: " + std::to_string(promptGame->score));
			body->text("Submit it online?");
			showButton(confirmButton, "YES", CONFIRM_COLOR);
			showButton(cancelButton, "NO", CANCEL_COLOR);
			break;
		case Phase::EnterName:
			title->text("NAME (A-Z, 0-9)");
			showButton(confirmButton, "SUBMIT", CONFIRM_COLOR);
			showButton(cancelButton, "CANCEL", CANCEL_COLOR);
			break;
		case Phase::Waiting:
			title->text("SUBMITTING...");
			break;
		case Phase::Result: // title/body filled in by the caller
			showButton(cancelButton, "CLOSE", NEUTRAL_COLOR);
			break;
		}
	}

	void showResult(const BoardResult& result) {
		setPhase(Phase::Result);
		if (!result) {
			title->text("COULD NOT SUBMIT");
			body->text(result.error());
			return;
		}

		title->text(std::format("TOP {}", SCOREBOARD_TOP_COUNT));
		std::string lines;
		for (std::size_t i = 0; i < result->size(); ++i) {
			const ScoreEntry& entry = (*result)[i];
			lines += std::format("{}. {:<{}} {:>9}\n", i + 1, entry.name, SCOREBOARD_NAME_MAX, entry.score);
		}
		body->text(lines.empty() ? "No scores yet" : lines);
	}

	void accept(GameState& game) {
		if (!game.scoreboard)
			game.scoreboard = std::make_unique<ScoreboardClient>();
		setPhase(Phase::EnterName);
	}

	void submit(GameState& game) {
		if (!game.scoreboard || !ScoreboardClient::isValidName(nameBox->value()))
			return;

		try {
			pending = std::async(std::launch::async,
				[client = *game.scoreboard, name = nameBox->value(), score = game.score]() -> BoardResult {
					if (auto sent = client.submitScore(name, score); !sent)
						return std::unexpected(sent.error());
					return client.topScores();
				});
			setPhase(Phase::Waiting);
		}
		catch (const std::system_error&) { // couldn't start a thread
			showResult(std::unexpected("could not start request"));
		}
	}

	void confirm(GameState& game) {
		switch (phase) {
		case Phase::Ask: accept(game); break;
		case Phase::EnterName: submit(game); break;
		case Phase::Result: setPhase(Phase::Hidden); break;
		default: break;
		}
	}

	void cancel() {
		if (phase != Phase::Waiting)
			setPhase(Phase::Hidden);
	}

	void onConfirmClicked(cenui::Frame*) {
		if (promptGame)
			confirm(*promptGame);
	}

	void onCancelClicked(cenui::Frame*) {
		cancel();
	}

	cenui::Frame* makeButton(const std::string& id, float anchorX, cenui::ClickCallback onClick) {
		return (new cenui::Frame(id))
			->visible(false)
			->anchor({anchorX, 1.0f})
			->position({{anchorX, 1.0f}, {0, 0}})
			->size({{0.48f, 0}, {0, 36}})
			->outlineColor(TEXT_COLOR)
			->text("", gameFont, 14)
			->textColor(TEXT_COLOR)
			->onClick(onClick);
	}
}

void initScorePrompt(GameState& game) {
	promptGame = &game;

	cenui::Frame* container = game.ui.root()->append(new cenui::Frame("scorePromptContainer"))
		->size({{1.0f, 1.0f}, {0, 0}})
		->bgColor(CLEAR)
		->outlineThickness(0);

	panel = container->append(new cenui::Frame("scorePrompt"))
		->visible(false)
		->anchor({0.5f, 0.5f})
		->position({{0.5f, 0.5f}, {0, 25}})
		->size({{0, 0}, {440, 210}})
		->bgColor(PANEL_COLOR)
		->outlineColor(TEXT_COLOR)
		->padding({10.0f, 10.0f, 10.0f, 10.0f});

	title = panel->append(new cenui::Frame("title"))
		->size({{1.0f, 0}, {0, 24}})
		->bgColor(CLEAR)
		->outlineThickness(0)
		->text("", gameFont, 16)
		->textColor(TEXT_COLOR);

	body = panel->append(new cenui::Frame("body"))
		->position({{0, 0}, {0, 34}})
		->size({{1.0f, 0}, {0, 110}})
		->bgColor(CLEAR)
		->outlineThickness(0)
		->text("", gameFont, 12)
		->textColor(TEXT_COLOR);

	cancelButton = panel->append(makeButton("cancel", 1.0f, onCancelClicked));
	confirmButton = panel->append(makeButton("confirm", 0.0f, onConfirmClicked));

	nameField = panel->append(new cenui::Frame("nameField"))
		->visible(false)
		->anchor({0.5f, 0.5f})
		->position({{0.5f, 0}, {0, 89}})
		->size({{0, 0}, {240, 36}})
		->bgColor(FIELD_COLOR)
		->outlineColor(TEXT_COLOR)
		->padding({10.0f, 10.0f, 0, 0})
		->text("", gameFont, 16)
		->textColor(TEXT_COLOR)
		->textXAlignment(cenui::XAlignment::LEFT);
	nameBox.emplace(nameField, SCOREBOARD_NAME_MAX, [](char c) {
		return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
	});
}

void openScorePrompt(GameState&) {
	setPhase(Phase::Ask);
}

void closeScorePrompt(GameState&) {
	setPhase(Phase::Hidden);
}

bool scorePromptOpen() {
	return phase != Phase::Hidden;
}

void tickScorePrompt(GameState&) {
	using namespace std::chrono_literals;
	if (phase != Phase::Waiting || !pending.valid() || pending.wait_for(0s) != std::future_status::ready)
		return;

	try {
		showResult(pending.get());
	}
	catch (const std::exception&) {
		showResult(std::unexpected("unexpected error"));
	}
}

void keyPressedScorePrompt(GameState& game, const sf::Event::KeyPressed* event) {
	switch (event->scancode) {
	case sf::Keyboard::Scancode::Enter:
	case sf::Keyboard::Scancode::NumpadEnter:
		confirm(game);
		break;
	case sf::Keyboard::Scancode::Escape:
		cancel();
		break;
	default:
		break;
	}
}

void textEnteredScorePrompt(GameState&, const sf::Event::TextEntered* event) {
	if (phase == Phase::EnterName)
		nameBox->handleTextEntered(*event);
}
